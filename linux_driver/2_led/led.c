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

#define LED_MAJOR 200
#define LED_NAME "myx_led"
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

static char readbuffer[1];
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
static int led_open(struct inode *inode,struct file *filp)
{
    return 0;
}

static int led_release(struct inode *inode,struct file *filp)
{
    return 0;
}
static int led_read(struct file *filp, char __user *buf,size_t cnt, loff_t *offt)
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
static ssize_t led_write(struct file *filp,const char __user *buf,size_t cnt, loff_t *offt)
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

//字符设备操作集合
static const struct file_operations led_fop = 
{
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .write = led_write,
    .read = led_read,
};

//加载
static int __init led_init(void)
{
    int ret = 0;
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
    ret = register_chrdev(LED_MAJOR, LED_NAME, &led_fop);
    if(ret < 0)
    {
        printk("led驱动注册失败！\r\n");
        return -EIO;
    }
    printk("led驱动加载！\r\n");
    return 0;
}

//卸载
static void __exit led_exit(void)
{    
    //取消地址映射
    iounmap(IMX6_CMM_CCGR1); //一个寄存器32bit=4byte
    iounmap(IMX6_SW_MUX_GPIO1_IO03);
    iounmap(IMX6_SW_PAD_GPIO1_IO03);
    iounmap(IMX6_GPIO1_GDIR);
    iounmap(IMX6_GPIO1_DR);

    //卸载字符设备驱动
    unregister_chrdev(LED_MAJOR,LED_NAME);
    printk("led驱动卸载！\r\n");
}


//注册驱动的加载和卸载
module_init(led_init);
module_exit(led_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



