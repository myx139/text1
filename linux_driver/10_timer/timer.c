#include <linux/types.h> 
#include <linux/kernel.h> 
#include <linux/delay.h> 
#include <linux/ide.h> 
#include <linux/init.h>
#include <linux/module.h> 
#include <linux/errno.h>  
#include <linux/gpio.h> 
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/of_gpio.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
/*
    gpioled
    {
        compatible = "gpio_led";
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_gpioled>;
        led-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>;
        status = "okay";
    };
*/
#define NAME "timer"
#define LED_ON 0
#define LED_OFF 1
#define IMX6_GPIO1_DR 0
int ret = 0;
//设备结构体
struct timer
{
    struct cdev cdev;
    dev_t devid; 
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node * lednd;
    struct timer_list timer0;
    const char * ledcom;
    int gpio_led;
}timer;

static void set_led(unsigned char sta)
{
    int temp = 0;
    if(sta == LED_OFF)
    {
        gpio_set_value(timer.gpio_led,1);
    }
    else if (sta == LED_ON)
    {
        gpio_set_value(timer.gpio_led,0);
    }
}
static int get_led(void)
{
    return 0;
}

static int timer_open(struct inode *inode, struct file *filp)
{

    return 0;
}
static ssize_t timer_read(struct file *filp, char __user *buf,
                                size_t cnt, loff_t *offt)
{
    static int data[1];
    data[0] = get_led();
    /* 向用户空间发送数据 */
    ret = copy_to_user(buf, data, cnt);
    if(ret == 0)
    {
    //printk("kernel senddata ok!\r\n");
    }
    else
    {
    printk("kernel senddata failed!\r\n");
    }
    return 0;

}

static ssize_t timer_write(struct file *filp, const char __user *buf,
                                size_t cnt, loff_t *offt)
{
    unsigned char databuf[1];
    ret = copy_from_user(databuf,buf,cnt);
    if(ret < 0)
    {
        printk("写入数据失败！\r\n");
        return -EFAULT;
    }
    else
    {
        set_led(databuf[0]);
    }
    return 0;
}
static int timer_release(struct inode *inode, struct file *filp)
{
    return 0;
}

const struct file_operations timer_fops=
{
    .owner = THIS_MODULE,
    .open = timer_open,
    .read = timer_read,
    .write = timer_write,
    .release = timer_release,
};
static void gpio_led_init(void)
{
    //获取设备结点
    timer.lednd = of_find_node_by_path("/gpioled");
    if (timer.lednd == NULL)
    {
        ret = -EINVAL;
    }
    /*获取com属性*/
    ret =  of_property_read_string(timer.lednd,"compatible",&timer.ledcom);
    if(ret < 0)
    {
        printk("fail_read_str\r\n");
    }
    else
    {
        printk("compatible = %s\r\n",timer.ledcom);
    }
    /*ledgpio的相关属性*/
    timer.gpio_led = of_get_named_gpio(timer.lednd,"led-gpios",0);
    if(timer.gpio_led < 0)
    {
        printk("find gpio filed!\r\n");
        return -EINVAL;
    }
    printk("led gpio num : %d\r\n",timer.gpio_led);
    //申请io
    ret = gpio_request(timer.gpio_led,"led_gpio");
    if(ret)
    {
        printk("filed to request gpio\r\n");
    }
    //使用io
    ret = gpio_direction_output(timer.gpio_led, 1);
    if(ret < 0)
    {
    printk("can't set gpio!\r\n");
    }
    gpio_set_value(timer.gpio_led,1);
}
static void gpio_led_exit(void)
{
    gpio_set_value(timer.gpio_led,1);
    //释放io
    gpio_free(timer.gpio_led);
}
static void timer0_server(unsigned long arg)
{
    static int i = 0;
    gpio_set_value(timer.gpio_led,i);
    i = !i;
    mod_timer(&timer.timer0,jiffies + msecs_to_jiffies(500));
}
static void timer0_init(void)
{
    init_timer(&timer.timer0);
    timer.timer0.function = timer0_server;
    timer.timer0.expires = jiffies + msecs_to_jiffies(500);
    timer.timer0.data = (unsigned long)&timer;
    add_timer(&timer.timer0);
}
static void timer0_exit(void)
{
    del_timer(&timer.timer0);
}
static int __init timer_init(void)
{
    //申请设备号
    timer.major = 0;
    if(timer.major)
    {
        timer.devid = MKDEV(timer.major,timer.minor);
        ret = register_chrdev_region(timer.devid,1,NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&timer.devid,0,1,NAME);
        timer.major = MAJOR(timer.devid);
        timer.minor = MINOR(timer.devid);
    }
    if(ret < 0)
    {
        printk("申请设备号失败！\r\n");
        return -1;
    }
    printk("major = %d,minor = %d\r\n",timer.major,timer.minor);

    //创建设备
    timer.cdev.owner = THIS_MODULE;
    cdev_init(&timer,&timer_fops);
    ret = cdev_add(&timer.cdev,timer.devid,1);
    if(ret <0)
    {
        printk("向系统添加字符设备失败\r\n");
    }
    //自动创建设备结点
    timer.class = class_create(THIS_MODULE,NAME);
    if(IS_ERR(timer.class))
    {
        return PTR_ERR(timer.class);
    }
    timer.device = device_create(timer.class,NULL,timer.devid,NULL,NAME);
    if(IS_ERR(timer.device))
    {
        return PTR_ERR(timer.device);
    }
    gpio_led_init();
    timer0_init();
    return 0;
}
static void __exit timer_exit(void)
{
    gpio_led_exit();
    timer0_exit();
     /*删除字符设备*/
    cdev_del(&timer.cdev);
    printk("删除字符设备\r\n");
    /*注销设备号*/
    unregister_chrdev_region(timer.devid, 1);
    printk("注销设备号\r\n");
    /*先摧毁设备*/
    device_destroy(timer.class, timer.devid);
    printk("先摧毁设备\r\n");
    /*摧毁类*/
    class_destroy(timer.class);
    printk("摧毁类\r\n");
    printk("驱动已卸载！\r\n");

}

module_init(timer_init);
module_exit(timer_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



