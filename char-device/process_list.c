#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>     
#include <linux/cdev.h>     
#include <linux/semaphore.h>  
#include <asm/uaccess.h>    
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/slab.h>

//Variables
struct task_struct *task;
int len_data = 0;
struct semaphore sem;
int ret;
char *buf_ptr;

//Macro
#define DEVICE_NAME "process_list"

//State
char * returnState(long n) {
    switch (n) {
        case 0: 
            return "TASK_RUNNING";
            break;
        case 1: 
            return "TASK_INTERRUPTIBLE";
            break;
        case 2: 
            return "TASK_UNINTERRUPTIBLE";
            break;
        case 4: 
            return "__TASK_STOPPED";
            break;
        case 8: 
            return "__TASK_TRACED";
            break;
        case 16: 
            return "EXIT_DEAD";
            break;
        case 32: 
            return "EXIT_ZOMBIE";
            break;
        case 48: 
            return "EXIT_ZOMBIE, EXIT_DEAD";
            break;
        case 64: 
            return "TASK_DEAD";
            break;
        case 128: 
            return "TASK_WAKEKILL";
            break;
        case 256: 
            return "TASK_WAKING";
            break;
        case 512: 
            return "TASK_PARKED";
            break;
        case 1024: 
            return "TASK_NOLOAD";
            break;
        case 2048: 
            return "TASK_NEW";
            break;
        case 4096: 
            return "TASK_STATE_MAX";
            break;
        case 130: 
            return "TASK_WAKEKILL,TASK_UNINTERRUPTIBLE";
            break;
        case 132: 
            return "TASK_WAKEKILL,__TASK_STOPPED";
            break;
        case 136: 
            return "TASK_WAKEKILL,__TASK_TRACED";
            break;
        case 1026: 
            return "TASK_UNINTERRUPTIBLE,TASK_NOLOAD";
            break;
        case 3: 
            return "TASK_INTERRUPTIBLE,TASK_UNINTERRUPTIBLE";
            break;
        case 15:
            return "TASK_NORMAL,__TASK_STOPPED,__TASK_TRACED";
            break;
        case 63:
            return "TASK_RUNNING,TASK_INTERRUPTIBLE,TASK_UNINTERRUPTIBLE,__TASK_STOPPED,__TASK_TRACED,EXIT_ZOMBIE,EXIT_DEAD";
            break;
        default:            
            return "TASK_UNRUNNABLE";
            break;
    }
}


//Functions
//Device Open
int device_open(struct inode *inode, struct file *filp) {
    struct task_struct *p;
    pid_t process_id, parent_id;
    volatile long process_state;
    unsigned int cpu_number;

    char str_process_id[64];
    char str_parent_id[64];
    char str_process_state[64];
    char str_cpu_number[64];
    int count  = 0;
    int temp_len = 0;
    int tmpCount = 0;
    printk(KERN_ALERT "process_list : Opened Device...!!!\n");

    buf_ptr = kmalloc(10 * sizeof(char), GFP_KERNEL);

    if(!buf_ptr) {
        printk(KERN_ALERT "Unable to allocate memory...!!!\n");
    }

    for_each_process(p) {
        process_id = p->pid;
        parent_id = p->parent->pid;
        process_state = p->state;
        cpu_number = task_cpu(p);

        sprintf(str_process_id, "PID=%u ", process_id);
        sprintf(str_parent_id, "PPID=%u ", parent_id);
        sprintf(str_cpu_number, "CPU=%u ", cpu_number);
        sprintf(str_process_state, "STATE=%s\n", returnState(process_state));

        tmpCount = count;
        temp_len = strlen(str_process_id);
        buf_ptr = krealloc(buf_ptr, (count + temp_len) * sizeof(char), GFP_KERNEL);
        if(!buf_ptr) {
            printk(KERN_ALERT "Unable to allocate memory...!!!\n");
        }
        strcpy(buf_ptr + count, str_process_id);
        count = count + strlen(str_process_id);

        temp_len = strlen(str_parent_id);
        buf_ptr = krealloc(buf_ptr, (count + temp_len) * sizeof(char), GFP_KERNEL);
        if(!buf_ptr) {
            printk(KERN_ALERT "Unable to allocate memory...!!!\n");
        }
        strcpy(buf_ptr + count, str_parent_id);
        count = count + strlen(str_parent_id);

        temp_len = strlen(str_cpu_number);
        buf_ptr = krealloc(buf_ptr, (count + temp_len) * sizeof(char), GFP_KERNEL);
        if(!buf_ptr) {
            printk(KERN_ALERT "Unable to allocate memory...!!!\n");
        }
        strcpy(buf_ptr + count, str_cpu_number);
        count = count + strlen(str_cpu_number);

        temp_len = strlen(str_process_state);
        buf_ptr = krealloc(buf_ptr, (count + temp_len) * sizeof(char), GFP_KERNEL);
        if(!buf_ptr) {
            printk(KERN_ALERT "Unable to allocate memory...!!!\n");
        }
        strcpy(buf_ptr + count, str_process_state);
        count = count + strlen(str_process_state);

    }
    buf_ptr = krealloc(buf_ptr, (count + 1) * sizeof(char), GFP_KERNEL);
    if(!buf_ptr) {
        printk(KERN_ALERT "Unable to allocate memory...!!!\n");
    }
    *(buf_ptr + count) = '\0';
    len_data = 0;
    return 0;
}

//Device Read
ssize_t device_read(struct file* filp, char* bufStoreData, size_t bufCount, loff_t* curOffset) {
    int sample = 0;
    if(len_data == strlen(buf_ptr)) {
        return 0;
    }
    if((len_data + bufCount) < strlen(buf_ptr)) {
        sample = bufCount;   
        if(copy_to_user(bufStoreData, buf_ptr + len_data, sample)) {
            return -1;
        }
    } else {
        sample = strlen(buf_ptr) - len_data; 
        if(copy_to_user(bufStoreData, buf_ptr + len_data, sample)) {
            return -1;
        }
    }
    len_data = len_data + sample;
    return sample;
}

//Device Close
int device_close(struct inode *inode, struct file* filp) {
    printk(KERN_ALERT "process_list : Closed device...!!!\n");
    kfree(buf_ptr);
    return 0;
}

//File Struct
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_close,
    .read = device_read 
};

//Device Struct
static struct miscdevice process_list = {
    .minor = MISC_DYNAMIC_MINOR, 
    .name = "process_list",
    .fops = &fops
};

//Driver Entry
static int __init driver_entry(void) {
    //anything other than zero
    int flag = 0;
    flag = misc_register(&process_list);
    if(flag != 0) {
        printk(KERN_ALERT "process_list : Error while registering device...!!!\n");
        return 1;
    }
    printk(KERN_ALERT "process_list : Device Registered Successfully...!!!\n");
    return 0; 
}

//Driver Exit
static void __exit driver_exit(void) {
    misc_deregister(&process_list);
    printk(KERN_ALERT "process_list : Device Deregistered Successfully...!!!\n");
}

//Module Entry and Exit
module_init(driver_entry);
module_exit(driver_exit);
