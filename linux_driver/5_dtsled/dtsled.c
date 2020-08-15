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
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#define LED_OFF 1
#define LED_ON 0
#define NAME "dtsled"
//相关寄存器物理地址
/*
 CMM_CCGR1_BASE (0X020C406C)   //时钟基地址
 SW_MUX_GPIO1_IO03_BASE (0X020E0068) //设置复用为GPIO1-IO03
 SW_PAD_GPIO1_IO03_BASE (0X020E02F4) //设置电气属性
 GPIO1_GDIR_BASE (0X0209C004)
 GPIO1_DR_BASE (0X0209C000)
*/
//地址映射后的虚拟地址
static void __iomem *IMX6_CMM_CCGR1;
static void __iomem *IMX6_SW_MUX_GPIO1_IO03;
static void __iomem *IMX6_SW_PAD_GPIO1_IO03;
static void __iomem *IMX6_GPIO1_GDIR;
static void __iomem *IMX6_GPIO1_DR;

int ret = 0;
u32 * val;
/*设备结构体*/
struct dtsled
{
    struct cdev cdev;
    dev_t devid;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node * nd;
    const char * com;
} MYX_dtsled;
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

static int dtsled_open(struct inode *inode, struct file *filp)
{

    return 0;
}
static ssize_t dtsled_read(struct file *filp, char __user *buf,
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
static ssize_t dtsled_write(struct file *filp, const char __user *buf,
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
static int dtsled_release(struct inode *inode, struct file *filp)
{
    return 0;
}

const struct file_operations dtsled_fops=
{
    .owner = THIS_MODULE,
    .open = dtsled_open,
    .read = dtsled_read,
    .write = dtsled_write,
    .release = dtsled_release,
};

static int __init dtsled_init(void)
{
    /*从设备树获取信息*/
    /*找到结点*/
    u32 elesize = 0;
    int i,temp;
    MYX_dtsled.nd = of_find_node_by_path("/MYX_led");
    if (MYX_dtsled.nd == NULL)
    {
        ret = -EINVAL;
        goto fail_find;
    }
    /*获取com属性*/
    ret =  of_property_read_string(MYX_dtsled.nd,"compatible",&MYX_dtsled.com);
    if(ret < 0)
    {
        goto fail_read_str;
    }
    else
    {
        printk("compatible = %s\r\n",MYX_dtsled.com);
    }
    /*读取寄存器信息*/
    elesize = of_property_count_elems_of_size(MYX_dtsled.nd,"reg",4);
    if(elesize < 0)
    {
        goto fail_read_ele;
    }
    else
    {
        printk("reg have = %d\r\n",elesize);
    }
    val = kmalloc(elesize * sizeof(u32),GFP_KERNEL);
    if(val != NULL)
    {
        ret = of_property_read_u32_array(MYX_dtsled.nd,"reg",val,elesize);
        if(ret < 0)
        {
            goto fail_read_u32_array;
        }
        else
        {
            for(i = 0;i <= 9; i++)
            printk("val[%x] = %x\r\n",i,*(val+i));//打印第二个元素
        }
    }
    /*初始化led*/
    /*初始化led外设*/
    //地址映射
    IMX6_CMM_CCGR1          = ioremap(val[0],val[1]); //一个寄存器32bit=4byte
    IMX6_SW_MUX_GPIO1_IO03  = ioremap(val[2],val[3]);
    IMX6_SW_PAD_GPIO1_IO03  = ioremap(val[4],val[5]);
    IMX6_GPIO1_DR           = ioremap(val[6],val[7]);
    IMX6_GPIO1_GDIR         = ioremap(val[8],val[9]);

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
    set_led(LED_ON);

    /*申请设备号*/
    MYX_dtsled.major = 0;
    MYX_dtsled.minor = 0;
    if(MYX_dtsled.major) //给定主设备号
    {
        MYX_dtsled.devid = MKDEV(MYX_dtsled.major,MYX_dtsled.minor);
        ret = register_chrdev_region(MYX_dtsled.devid,1,NAME);
    }
    else                //没有给定设备号
    {
        ret = alloc_chrdev_region(&MYX_dtsled.devid,0,1,NAME);
        MYX_dtsled.major = MAJOR(MYX_dtsled.devid);
        MYX_dtsled.minor = MINOR(MYX_dtsled.devid);
    }
    if(ret < 0)
    {
        printk("注册新字符设备驱动失败！\r\n");
        return -1;
    }
    printk("major = %d,minor = %d\r\n",MYX_dtsled.major,MYX_dtsled.minor);
    /*添加设备*/
    MYX_dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&MYX_dtsled,&dtsled_fops);
    ret = cdev_add(&MYX_dtsled.cdev,MYX_dtsled.devid,1);
    if(ret <0)
    {
        printk("向系统添加字符设备失败\r\n");
    }

    MYX_dtsled.class = class_create(THIS_MODULE,NAME);
    if(IS_ERR(MYX_dtsled.class))
    {
        return PTR_ERR(MYX_dtsled.class);
    }
    MYX_dtsled.device = device_create(MYX_dtsled.class,NULL,MYX_dtsled.devid,NULL,NAME);
    if(IS_ERR(MYX_dtsled.device))
    {
        return PTR_ERR(MYX_dtsled.device);
    }

    return 0;
fail_read_u32_array:
fail_read_str:
fail_read_ele:
fail_find:
    return ret;
}
static void __exit dtsled_exit(void)
{
    val = NULL;
    kfree(val);
    iounmap(IMX6_CMM_CCGR1);
    iounmap(IMX6_SW_MUX_GPIO1_IO03);
    iounmap(IMX6_SW_PAD_GPIO1_IO03);
    iounmap(IMX6_GPIO1_DR);
    iounmap(IMX6_GPIO1_GDIR);
    /*删除字符设备*/
    cdev_del(&MYX_dtsled.cdev);
    printk("删除字符设备\r\n");
    /*注销设备号*/
    unregister_chrdev_region(MYX_dtsled.devid, 1);
    printk("注销设备号\r\n");
    /*先摧毁设备*/
    device_destroy(MYX_dtsled.class, MYX_dtsled.devid);
    printk("先摧毁设备\r\n");
    /*摧毁类*/
    class_destroy(MYX_dtsled.class);
    printk("摧毁类\r\n");
    printk("驱动已卸载！\r\n");

}

module_init(dtsled_init);
module_exit(dtsled_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



