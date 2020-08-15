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
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/device.h>
#define newchrdevname "ledchrdev"
#define LED_OFF 1
#define LED_ON 0
//相关寄存器物理地址
#define CMM_CCGR1_BASE (0X020C406C)   //时钟基地址
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068) //设置复用为GPIO1-IO03
#define SW_PAD_GPIO1_IO03_BASE (0X020E02F4) //设置电气属性
#define GPIO1_GDIR_BASE (0X0209C004)
#define GPIO1_DR_BASE (0X0209C000)

//地址映射后的虚拟地址
static void __iomem *IMX6_CMM_CCGR1;
static void __iomem *IMX6_SW_MUX_GPIO1_IO03;
static void __iomem *IMX6_SW_PAD_GPIO1_IO03;
static void __iomem *IMX6_GPIO1_GDIR;
static void __iomem *IMX6_GPIO1_DR;


/*设备结构体*/
struct  newchrdev
{
    struct cdev cdev; //字符设备
    dev_t devid;  //设备号
    struct class *class; //类
    struct  device *device; //设备
    int major;     //主设备号
    int minor;      //次设备号
}   ledchrdev;

static void set_led(unsigned char sta)
{
    int temp = 0;
    if(sta == LED_OFF)
    {
        temp = readl(IMX6_GPIO1_DR);
        temp |= (1<<3);
        writel(temp,IMX6_GPIO1_DR); //输出电平
    }
    else if (sta == LED_ON)
    {
        temp = readl(IMX6_GPIO1_DR);
        temp &= ~(1<<3);
        writel(temp,IMX6_GPIO1_DR); //输出电平 
    }
}
static int get_led(void)
{
    int temp = 0;
    temp = readl(IMX6_GPIO1_DR);
    return (temp&0x8)>>3;
}

static int ledchrdev_open(struct inode *inode, struct file *filp)
{

    return 0;
}
static ssize_t ledchrdev_read(struct file *filp, char __user *buf,
                                size_t cnt, loff_t *offt)
{
    int ret = 0;
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
static ssize_t ledchrdev_write(struct file *filp, const char __user *buf,
                                size_t cnt, loff_t *offt)
{
    int ret = 0;
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
static int ledchrdev_release(struct inode *inode, struct file *filp)
{
    return 0;
}
const struct file_operations ledchrdev_fops=
{
    .owner = THIS_MODULE,
    .open = ledchrdev_open,
    .read = ledchrdev_read,
    .write = ledchrdev_write,
    .release = ledchrdev_release,
};

static int __init newchrdev_init(void)
{
    int ret = 0;
    /*初始化led*/
    int temp = 0;
    /*初始化led外设*/
        //地址映射
        IMX6_CMM_CCGR1 = ioremap(CMM_CCGR1_BASE,4); //一个寄存器32bit=4byte
        IMX6_SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE,4);
        IMX6_SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE,4);
        IMX6_GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE,4);
        IMX6_GPIO1_DR = ioremap(GPIO1_DR_BASE,4);
        //打开时钟
        temp = readl(IMX6_CMM_CCGR1); //读
        temp &= ~(3<<26);            //改    先对26 27bit清零
        temp |= (3<<26);
        writel(temp,IMX6_CMM_CCGR1); //写
        //配置IO
        writel(0x5,IMX6_SW_MUX_GPIO1_IO03); //设置复用
        writel(0x10b0,IMX6_SW_PAD_GPIO1_IO03); //设置电器属性

        temp = readl(IMX6_GPIO1_GDIR); //读
        temp &= ~(1<<3);                //改
        temp |= (1<<3);
        writel(temp,IMX6_GPIO1_GDIR); //写设置为输出
        set_led(LED_OFF);


    /*注册字符设备*/
    if(ledchrdev.major)     //如果给定主设备号
    {
        ledchrdev.devid = MKDEV(ledchrdev.major,0);
        ret = register_chrdev_region(ledchrdev.devid, 1, newchrdevname);
    }
    else
    {
        ret = alloc_chrdev_region(&ledchrdev.devid, 0, 1, newchrdevname);
        ledchrdev.major = MAJOR(ledchrdev.devid);
        ledchrdev.minor = MINOR(ledchrdev.devid);
    }
    if(ret < 0)
    {
        printk("注册新字符设备驱动失败！\r\n");
        return -1;
    }
    printk("major = %d,minor = %d\r\n",ledchrdev.major,ledchrdev.minor);

    /*注册字符设备*/
    ledchrdev.cdev.owner = THIS_MODULE;
    cdev_init(&ledchrdev.cdev, &ledchrdev_fops);
    ret =  cdev_add(&ledchrdev.cdev, ledchrdev.devid, 1);
    if(ret < 0)
    {
        printk("向系统添加字符设备失败\r\n");
    }
    /*自动创建设备节点*/
    ledchrdev.class = class_create(THIS_MODULE,newchrdevname);
    if(IS_ERR(ledchrdev.class))
    {
        return PTR_ERR(ledchrdev.class);
    }
    ledchrdev.device = device_create(ledchrdev.class,NULL,ledchrdev.devid,NULL,newchrdevname);
    if(IS_ERR(ledchrdev.device))
    {
        return PTR_ERR(ledchrdev.device);
    }
    return 0;
}
static void __exit newchrdev_exit(void)
{
    /*删除字符设备*/
    cdev_del(&ledchrdev.cdev);
    /*注销设备号*/
    unregister_chrdev_region(ledchrdev.devid, 1);
    /*先摧毁设备*/
    device_destroy(ledchrdev.class, ledchrdev.devid);
    /*摧毁类*/
    class_destroy(ledchrdev.class);
    printk("驱动已卸载！\r\n");
}


module_init(newchrdev_init);
module_exit(newchrdev_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



