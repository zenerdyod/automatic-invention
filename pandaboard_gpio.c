/**
 * @file    : pandaboard_gpio.c
 * @Brief   : This file contains the source code for driving a gpio
 *          : for pandaboard.
 **/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/kthread.h>

typedef struct{
    int status, rate;
}led_t;

#define SUCCESS 0
#define FAILURE -1
#define OFF     _IOR('q', 0, led_t *)
#define ON      _IOR('q', 1, led_t *)
#define BLINK   _IOR('q', 2, led_t *)
#define PERIOD  1000
#define FIRST_MINOR    0
#define MINOR_CNT    1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zenerdyod");
MODULE_DESCRIPTION("LED DRIVER");
MODULE_VERSION("0.1");
led_t led_dev;
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
struct task_struct *task;

int status = ON, rate = PERIOD;
int ledOn = false;
int gpioLED = 8;

static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg){
    switch(cmd){
        case OFF:
            printk(KERN_INFO "LED OFF\n");
            status = OFF;
            break;
        case ON:
            printk(KERN_INFO "LED ON\n");
            status = ON;
            break;
        case BLINK:
            printk(KERN_INFO "LED BLINK\n");
            if (copy_from_user(&led_dev, (led_t *)arg, sizeof(led_t))){
                return -EACCES;
            }
            status = BLINK;
            rate = led_dev.rate;
            break;
        default:
            return -EINVAL;
    }
    return SUCCESS;
}

static struct file_operations led_file = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = my_ioctl
};

static int flash(void *arg){
       printk(KERN_INFO "Thread has started running \n");
       while(!kthread_should_stop()){
            set_current_state(TASK_RUNNING);
            if (status == BLINK) ledOn = !ledOn;
            else if (status == OFF) ledOn = false;
            else ledOn = true;
            gpio_set_value(gpioLED, ledOn);
            set_current_state(TASK_INTERRUPTIBLE);
            msleep(rate/2);
        }
        printk(KERN_INFO "Thread has run to completion \n");
        return SUCCESS;
}


static int __init led_driver_init(void){
    int ret;
    struct device *dev_ret;

    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "led_ioctl")) < 0) {
        return ret;
    }
    cdev_init(&c_dev, &led_file);

    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0){
        return ret;
    }
    if (IS_ERR(cl = class_create(THIS_MODULE, "char"))){
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "led_driver"))){
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }
    gpio_request(gpioLED, "led_init");
    gpio_direction_output(gpioLED, ledOn);
    gpio_export(gpioLED, false);
    task = kthread_run(flash, NULL, "LED_flash_thread");
    if(IS_ERR(task)){
        printk(KERN_ALERT "EBB LED: failed to create the task\n");
        return PTR_ERR(task);
    }
    return SUCCESS;
}

static void __exit led_driver_exit(void){
    kthread_stop(task);
    gpio_set_value(gpioLED, OFF);
    gpio_unexport(gpioLED);
    gpio_free(gpioLED);
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}

module_init(led_driver_init);
module_exit(led_driver_exit);
