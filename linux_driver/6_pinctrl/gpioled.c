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
#define NAME "gpioled"
#define LED_ON 0
#define LED_OFF 1
#define IMX6_GPIO1_DR 0
int ret = 0;
//设备结构体
struct gpioled
{
    struct cdev cdev;
    dev_t devid; 
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node * nd;
    const char * com;
    int gpio_led;
}gpioled;

static void set_led(unsigned char sta)
{
    int temp = 0;
    if(sta == LED_OFF)
    {
        gpio_set_value(gpioled.gpio_led,1);
    }
    else if (sta == LED_ON)
    {
        gpio_set_value(gpioled.gpio_led,0);
    }
}
static int get_led(void)
{
    int temp = 0;
    temp = readl(IMX6_GPIO1_DR);
    return (temp&0x8)>>3;
}

static int gpioled_open(struct inode *inode, struct file *filp)
{

    return 0;
}
static ssize_t gpioled_read(struct file *filp, char __user *buf,
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

static ssize_t gpioled_write(struct file *filp, const char __user *buf,
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
static int gpioled_release(struct inode *inode, struct file *filp)
{
    return 0;
}

const struct file_operations gpioled_fops=
{
    .owner = THIS_MODULE,
    .open = gpioled_open,
    .read = gpioled_read,
    .write = gpioled_write,
    .release = gpioled_release,
};

static int __init gpioled_init(void)
{
    //申请设备号
    gpioled.major = 0;
    if(gpioled.major)
    {
        gpioled.devid = MKDEV(gpioled.major,gpioled.minor);
        ret = register_chrdev_region(gpioled.devid,1,NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&gpioled.devid,0,1,NAME);
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }
    if(ret < 0)
    {
        printk("申请设备号失败！\r\n");
        return -1;
    }
    printk("major = %d,minor = %d\r\n",gpioled.major,gpioled.minor);

    //创建设备
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled,&gpioled_fops);
    ret = cdev_add(&gpioled.cdev,gpioled.devid,1);
    if(ret <0)
    {
        printk("向系统添加字符设备失败\r\n");
    }
    //自动创建设备结点
    gpioled.class = class_create(THIS_MODULE,NAME);
    if(IS_ERR(gpioled.class))
    {
        return PTR_ERR(gpioled.class);
    }
    gpioled.device = device_create(gpioled.class,NULL,gpioled.devid,NULL,NAME);
    if(IS_ERR(gpioled.device))
    {
        return PTR_ERR(gpioled.device);
    }
    //获取设备结点
    gpioled.nd = of_find_node_by_path("/gpioled");
    if (gpioled.nd == NULL)
    {
        ret = -EINVAL;
    }
    /*获取com属性*/
    ret =  of_property_read_string(gpioled.nd,"compatible",&gpioled.com);
    if(ret < 0)
    {
        printk("fail_read_str\r\n");
    }
    else
    {
        printk("compatible = %s\r\n",gpioled.com);
    }
    /*ledgpio的相关属性*/
    gpioled.gpio_led = of_get_named_gpio(gpioled.nd,"led-gpios",0);
    if(gpioled.gpio_led < 0)
    {
        printk("find gpio filed!\r\n");
        return -EINVAL;
    }
    printk("led gpio num : %d\r\n",gpioled.gpio_led);
    //申请io
    ret = gpio_request(gpioled.gpio_led,"led_gpio");
    if(ret)
    {
        printk("filed to request gpio\r\n");
    }
    //使用io
    ret = gpio_direction_output(gpioled.gpio_led, 1);
    if(ret < 0)
    {
    printk("can't set gpio!\r\n");
    }
    gpio_set_value(gpioled.gpio_led,1);
    return 0;
}
static void __exit gpioled_exit(void)
{
    gpio_set_value(gpioled.gpio_led,1);
    //释放io
    gpio_free(gpioled.gpio_led);
     /*删除字符设备*/
    cdev_del(&gpioled.cdev);
    printk("删除字符设备\r\n");
    /*注销设备号*/
    unregister_chrdev_region(gpioled.devid, 1);
    printk("注销设备号\r\n");
    /*先摧毁设备*/
    device_destroy(gpioled.class, gpioled.devid);
    printk("先摧毁设备\r\n");
    /*摧毁类*/
    class_destroy(gpioled.class);
    printk("摧毁类\r\n");
    printk("驱动已卸载！\r\n");

}

module_init(gpioled_init);
module_exit(gpioled_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



