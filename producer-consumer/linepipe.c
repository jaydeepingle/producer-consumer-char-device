#include <linux/init.h>
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
#include <linux/wait.h>
#include <linux/mutex.h> 
MODULE_LICENSE("DUAL BSD/GPL");

//Module Param
static int n = 2;
module_param(n, int, S_IRUGO);

//Variables
int count = 0, len_data = 0, ret = 0; 
struct task_struct *task;
struct semaphore sem;
char **words;
char **start, **end, **fixed_start, **fixed_end;

static DEFINE_MUTEX(mut);
static DEFINE_SEMAPHORE(full);
static DEFINE_SEMAPHORE(empty);

//Macro
#define DEVICE_NAME "linepipe"

//Functions
//Device Open
int device_open(struct inode *inode, struct file *filp) {
    printk(KERN_ALERT "linepipe : Opened Device...!!!\n");
    return 0;
}

//Device Read
ssize_t device_read(struct file* filp, char* bufStoreData, size_t bufCount, loff_t* curOffset) {
    int sample = 0;
    if(down_interruptible(&full) < 0) {
        printk(KERN_ALERT "Error while down operation...!!!\n");
        return -ERESTARTSYS;
    }
    if(mutex_lock_interruptible(&mut) < 0) {
        printk(KERN_ALERT "Error while mutex down...!!!\n");
        return -ERESTARTSYS;
    }
    sample = strlen(start[0]);   
    if(copy_to_user(bufStoreData, start[0], (sample + 1))) {
        return -1;
    }
    kfree(start[0]);
    printk(KERN_ALERT "Read -> Data: %s", bufStoreData);
    if(start == fixed_end) {
        start = fixed_start;
    } else {
        start = start + 1;
    }
    mutex_unlock(&mut);
    up(&empty);
    return sample;
}

//Device Write
ssize_t device_write(struct file* filp, const char* bufSourceData, size_t bufCount, loff_t* curOffset) {
    int data_length = 0;
    if(down_interruptible(&empty) < 0) {
        printk(KERN_ALERT "Error while down operation...!!!\n");
        return -ERESTARTSYS;
    }
    if(mutex_lock_interruptible(&mut) < 0) {
        printk(KERN_ALERT "Error while mutex down...!!!\n");
        return -ERESTARTSYS;
    }
    data_length = strlen(bufSourceData);
    end[0] = kmalloc((data_length + 1) * sizeof(char), GFP_KERNEL);
    if(!end[0]) {
        printk(KERN_ALERT "Unable to allocate memory...!!!\n");
    }
    end[0][data_length + 1] = '\0';
    ret = copy_from_user(end[0], bufSourceData, strlen(bufSourceData));
    if(ret > 0) {
        printk(KERN_ALERT "Operation failed");
        return -EFAULT;
    }
    if(end == fixed_end) {
        end = fixed_start;
    } else {
        end = end + 1;
    }
    mutex_unlock(&mut);
    up(&full);
    return data_length;
}

//Device Close
int device_close(struct inode *inode, struct file* filp) {
    return 0;
}

//File Struct
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_close,
    .read = device_read,
    .write = device_write
};

//Device Struct
static struct miscdevice linepipe = {
    .minor = MISC_DYNAMIC_MINOR, 
    .name = "linepipe",
    .fops = &fops
};

//Driver Entry
static int __init driver_entry(void) {
    int flag = 0;
    sema_init(&full, 0);
    sema_init(&empty, n);
    mutex_init(&mut);

    words = (char **)kmalloc(sizeof(char*) * n, GFP_KERNEL);
    if(!words) {
        printk(KERN_ALERT "Unable to allocate memory...!!!\n");
    }
    start = words;
    end = words;
    fixed_start = words;
    fixed_end = words + n;

    flag = misc_register(&linepipe);
    if(flag != 0) {
        printk(KERN_ALERT "linepipe : Error while registering device...!!!\n");
        return 1;
    }
    printk(KERN_ALERT "linepipe : Device Registered Successfully...!!!\n");
    return 0; 
}

//Driver Exit
static void __exit driver_exit(void) {
    if(words) {
        kfree(words);
    }
    misc_deregister(&linepipe);
    printk(KERN_ALERT "linepipe : Device Deregistered Successfully...!!!\n");
}

//Module Entry and Exit
module_init(driver_entry);
module_exit(driver_exit);
