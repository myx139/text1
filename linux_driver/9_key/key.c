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
	key
	{
		#address-cells = <1>;
		#size-cells = <1>;
		compatible = "atkalpha-key";
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_key>;
		key-gpio = <&gpio1 18 GPIO_ACTIVE_LOW>;
		status = "okay";	
	};
*/
#define NAME "key"

int ret = 0;
//设备结构体
struct keydev
{
    struct cdev cdev;
    dev_t devid; 
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node * nd;
    const char * com;
    int gpio_key;
    atomic_t key_value;
}key;
struct leddev
{
    struct device_node * lednd;
    const char * ledcom;
    int gpio_led;
}led;


static void key_IO_init(void)
{
    //获取设备结点
    key.nd = of_find_node_by_path("/key");
    if (key.nd == NULL)
    {
        ret = -EINVAL;
    }
    /*获取com属性*/
    ret =  of_property_read_string(key.nd,"compatible",&key.com);
    if(ret < 0)
    {
        printk("Fail to read str of key!\r\n");
    }
    else
    {
        printk("Compatible of key = %s\r\n",key.com);
    }
    /*keygpio的相关属性*/
    key.gpio_key = of_get_named_gpio(key.nd,"key-gpio",0);
    if(key.gpio_key < 0)
    {
        printk("Find key gpio filed!\r\n");
        return -EINVAL;
    }
    printk("key gpio num : %d\r\n",key.gpio_key);
    //申请io
    ret = gpio_request(key.gpio_key,"key_gpio");
    if(ret)
    {
        printk("Filed to request  key gpio\r\n");
    }
    //使用io
    ret = gpio_direction_input(key.gpio_key);
    if(ret < 0)
    {
    printk("Can't set key gpio!\r\n");
    }
    /********************LED*******************/
    led.lednd = of_find_node_by_path("/gpioled");
    if (led.lednd == NULL)
    {
        ret = -EINVAL;
    }
    ret = of_property_read_string(led.lednd,"compatible",&led.ledcom);
    if(ret < 0)
    {
        printk("Fail to read str of led!\r\n");
    }
    else
    {
        printk("Compatible of led = %s\r\n",led.ledcom);
    }
    led.gpio_led = of_get_named_gpio(led.lednd,"led-gpios",0);
    if(led.gpio_led < 0)
    {
        printk("Find led gpio filed!\r\n");
        return -EINVAL;
    }
    printk("led gpio num : %d\r\n",led.gpio_led);
    //申请io
    ret = gpio_request(led.gpio_led,"led_gpio");
    if(ret)
    {
        printk("Filed to request led gpio\r\n");
    }
    //使用io
    ret = gpio_direction_output(led.gpio_led,1);
    if(ret < 0)
    {
    printk("Can't set led gpio!\r\n");
    }
}

static int key_open(struct inode *inode, struct file *filp)
{
    key_IO_init();
    return 0;
}
static ssize_t key_read(struct file *filp, char __user *buf,
                                size_t cnt, loff_t *offt)
{
    int ret = 0;
    static int data[1];
    if(gpio_get_value(key.gpio_key)==0)
    {
        atomic_set(&key.key_value,0);
        gpio_set_value(led.gpio_led,0);
    }
    else
    {
        atomic_set(&key.key_value,1);
        gpio_set_value(led.gpio_led,1);
    }
    data[0] = atomic_read(&key.key_value);
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
    return 0;
}

static ssize_t beep_write(struct file *filp, const char __user *buf,
                                size_t cnt, loff_t *offt)
{
    unsigned char data[1];;
    copy_from_user(data,buf,cnt);
    if (data[0] == 1)
    {
        while (1)
        {
            gpio_set_value(led.gpio_led,0);
            mdelay(200);
            gpio_set_value(led.gpio_led,1);
            mdelay(200);
        }
        
    }
    return 0;
}
static int key_release(struct inode *inode, struct file *filp)
{
    return 0;
}

const struct file_operations gpiokey_fops=
{
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .write = beep_write,
    .release = key_release,
};

static int __init gpiokey_init(void)
{
    //申请设备号
    key.major = 0;
    if(key.major)
    {
        key.devid = MKDEV(key.major,key.minor);
        ret = register_chrdev_region(key.devid,1,NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&key.devid,0,1,NAME);
        key.major = MAJOR(key.devid);
        key.minor = MINOR(key.devid);
    }
    if(ret < 0)
    {
        printk("申请设备号失败！\r\n");
        return -1;
    }
    printk("major = %d,minor = %d\r\n",key.major,key.minor);

    //创建设备
    key.cdev.owner = THIS_MODULE;
    cdev_init(&key,&gpiokey_fops);
    ret = cdev_add(&key.cdev,key.devid,1);
    if(ret <0)
    {
        printk("向系统添加字符设备失败\r\n");
    }
    //自动创建设备结点
    key.class = class_create(THIS_MODULE,NAME);
    if(IS_ERR(key.class))
    {
        return PTR_ERR(key.class);
    }
    key.device = device_create(key.class,NULL,key.devid,NULL,NAME);
    if(IS_ERR(key.device))
    {
        return PTR_ERR(key.device);
    }
    return 0;
}
static void __exit gpiokey_exit(void)
{
    gpio_set_value(key.gpio_key,1);
    //释放io
    gpio_free(key.gpio_key);
     /*删除字符设备*/
    cdev_del(&key.cdev);
    printk("删除字符设备\r\n");
    /*注销设备号*/
    unregister_chrdev_region(key.devid, 1);
    printk("注销设备号\r\n");
    /*先摧毁设备*/
    device_destroy(key.class, key.devid);
    printk("先摧毁设备\r\n");
    /*摧毁类*/
    class_destroy(key.class);
    printk("摧毁类\r\n");
    printk("驱动已卸载！\r\n");

}

module_init(gpiokey_init);
module_exit(gpiokey_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



