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
	beep
	{
		#address-cells = <1>;
		#size-cells = <1>;
		compatible = "beep";
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_beep>;
		beep-gpio = <&gpio5 1 GPIO_ACTIVE_HIGH>;
		status = "okay";
	}; 
*/
#define NAME "beep"
#define BEEP_ON 0
#define BEEP_OFF 1
#define IMX6_GPIO1_DR 0
int ret = 0;
//设备结构体
struct beep
{
    struct cdev cdev;
    dev_t devid; 
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node * nd;
    const char * com;
    int gpio_beep;
}beep;

static void set_beep(unsigned char sta)
{
    int temp = 0;
    if(sta == BEEP_OFF)
    {
        gpio_set_value(beep.gpio_beep,1);
    }
    else if (sta == BEEP_ON)
    {
        gpio_set_value(beep.gpio_beep,0);
    }
}

static int gpiobeep_open(struct inode *inode, struct file *filp)
{

    return 0;
}
static ssize_t gpiobeep_read(struct file *filp, char __user *buf,
                                size_t cnt, loff_t *offt)
{
    return 0;

}

static ssize_t gpiobeep_write(struct file *filp, const char __user *buf,
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
        set_beep(databuf[0]);
    }
    return 0;
}
static int gpiobeep_release(struct inode *inode, struct file *filp)
{
    return 0;
}

const struct file_operations gpiobeep_fops=
{
    .owner = THIS_MODULE,
    .open = gpiobeep_open,
    .read = gpiobeep_read,
    .write = gpiobeep_write,
    .release = gpiobeep_release,
};

static int __init gpiobeep_init(void)
{
    //申请设备号
    beep.major = 0;
    if(beep.major)
    {
        beep.devid = MKDEV(beep.major,beep.minor);
        ret = register_chrdev_region(beep.devid,1,NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&beep.devid,0,1,NAME);
        beep.major = MAJOR(beep.devid);
        beep.minor = MINOR(beep.devid);
    }
    if(ret < 0)
    {
        printk("申请设备号失败！\r\n");
        return -1;
    }
    printk("major = %d,minor = %d\r\n",beep.major,beep.minor);

    //创建设备
    beep.cdev.owner = THIS_MODULE;
    cdev_init(&beep,&gpiobeep_fops);
    ret = cdev_add(&beep.cdev,beep.devid,1);
    if(ret <0)
    {
        printk("向系统添加字符设备失败\r\n");
    }
    //自动创建设备结点
    beep.class = class_create(THIS_MODULE,NAME);
    if(IS_ERR(beep.class))
    {
        return PTR_ERR(beep.class);
    }
    beep.device = device_create(beep.class,NULL,beep.devid,NULL,NAME);
    if(IS_ERR(beep.device))
    {
        return PTR_ERR(beep.device);
    }
    //获取设备结点
    beep.nd = of_find_node_by_path("/beep");
    if (beep.nd == NULL)
    {
        ret = -EINVAL;
    }
    /*获取com属性*/
    ret =  of_property_read_string(beep.nd,"compatible",&beep.com);
    if(ret < 0)
    {
        printk("fail_read_str\r\n");
    }
    else
    {
        printk("compatible = %s\r\n",beep.com);
    }
    /*ledgpio的相关属性*/
    beep.gpio_beep = of_get_named_gpio(beep.nd,"beep-gpio",0);
    if(beep.gpio_beep < 0)
    {
        printk("find gpio filed!\r\n");
        return -EINVAL;
    }
    printk("beep gpio num : %d\r\n",beep.gpio_beep);
    //申请io
    ret = gpio_request(beep.gpio_beep,"beep_gpio");
    if(ret)
    {
        printk("filed to request gpio\r\n");
    }
    //使用io
    ret = gpio_direction_output(beep.gpio_beep, 1);
    if(ret < 0)
    {
    printk("can't set gpio!\r\n");
    }
    gpio_set_value(beep.gpio_beep,1);
    return 0;
}
static void __exit gpiobeep_exit(void)
{
    gpio_set_value(beep.gpio_beep,1);
    //释放io
    gpio_free(beep.gpio_beep);
     /*删除字符设备*/
    cdev_del(&beep.cdev);
    printk("删除字符设备\r\n");
    /*注销设备号*/
    unregister_chrdev_region(beep.devid, 1);
    printk("注销设备号\r\n");
    /*先摧毁设备*/
    device_destroy(beep.class, beep.devid);
    printk("先摧毁设备\r\n");
    /*摧毁类*/
    class_destroy(beep.class);
    printk("摧毁类\r\n");
    printk("驱动已卸载！\r\n");

}

module_init(gpiobeep_init);
module_exit(gpiobeep_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



