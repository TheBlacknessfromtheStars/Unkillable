#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/pid.h>
#include <linux/version.h>

#define TAG "Unkillable"

#define LOGI(...) pr_info(TAG ": " __VA_ARGS__)
#define LOGE(...) pr_err(TAG ": " __VA_ARGS__)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Azure");
MODULE_VERSION("1.0");

static int target_pid = 0;
module_param(target_pid, int, 0444);
MODULE_PARM_DESC(target_pid, "PID of the process to protect from SIGKILL");

static int protect_process(void)
{
    struct task_struct *task;
    struct pid *pid_struct;
    
    if (target_pid <= 0) {
        LOGE("Invalid target PID: %d\n", target_pid);
        return -EINVAL;
    }
    
    pid_struct = find_get_pid(target_pid);
    if (!pid_struct) {
        LOGE("Cannot find process with PID: %d\n", target_pid);
        return -ESRCH;
    }
    
    task = get_pid_task(pid_struct, PIDTYPE_PID);
    if (!task) {
        LOGE("Cannot get task for PID: %d\n", target_pid);
        put_pid(pid_struct);
        return -ESRCH;
    }
    
    LOGI("Protecting process: %s (PID: %d)\n", task->comm, target_pid);
    
    spin_lock_irq(&task->sighand->siglock);
    
    task->sighand->action[SIGKILL - 1].sa.sa_handler = SIG_IGN;
    task->sighand->action[SIGKILL - 1].sa.sa_flags = 0;
    sigemptyset(&task->sighand->action[SIGKILL - 1].sa.sa_mask);
    
    spin_unlock_irq(&task->sighand->siglock);
    
    put_pid(pid_struct);
    put_task_struct(task);
    
    LOGI("Process protection enabled\n");

    return 0;
}

static void unprotect_process(void)
{
    struct task_struct *task;
    struct pid *pid_struct;
    
    if (target_pid <= 0) {
        LOGI("No protected process to restore\n");
        return;
    }
    
    pid_struct = find_get_pid(target_pid);
    if (!pid_struct) {
        LOGI("Protected process not found during cleanup\n");
        return;
    }
    
    task = get_pid_task(pid_struct, PIDTYPE_PID);
    if (task) {
        LOGI("Restoring signal handlers for process: %s (PID: %d)\n", 
                task->comm, target_pid);
        
        spin_lock_irq(&task->sighand->siglock);
        
        task->sighand->action[SIGKILL - 1].sa.sa_handler = SIG_DFL;
        
        spin_unlock_irq(&task->sighand->siglock);
        
        put_task_struct(task);
    }
    
    put_pid(pid_struct);
    target_pid = 0;
    LOGI("Process protection disabled\n");
}

static int __init unkillable_init(void)
{
    LOGI("module init\n");

    return protect_process();
}

static void __exit unkillable_exit(void)
{
    LOGI("module exit\n");
    unprotect_process();
}

module_init(unkillable_init);
module_exit(unkillable_exit);
