#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_ALERT */
#include <linux/init.h> /*Needed for __init */
#include <linux/printk.h> //printk头文件
#include <linux/fs.h> //file_operations结构体头文件
#define CHRDEVBASE_MAJOR 201
#define CHRDEVBASE_NAME "myx_1"
//打开驱动函数
static ssize_t chrdev_open(struct inode *inode, struct file *filp)
{
    return 0;
}
//驱动读函数
static ssize_t chrdev_read(struct file *filp, char __user *buf,
                                size_t cnt, loff_t *offt)
{
    return 0;
}
//驱动写函数
static ssize_t chrdev_write(struct file *filp,const char __user *buf,
                                size_t cnt, loff_t *offt)
{
    return 0;
}
//关闭驱动函数
static ssize_t chrdev_release(struct inode *inode, struct file *filp)
{
    return 0;
}
//设备操作函数结构体
static struct file_operations chrdevsettings=
{
    .owner = THIS_MODULE,
    .open = chrdev_open,
    .read = chrdev_read,
    .write = chrdev_write,
    .release = chrdev_release
};
//驱动入口函数，注册字符设备
static int   __init chrdev_init(void)
{
    /*
    参数1：主设备号
    参数2：设备名
    参数3：设备参数结构体
    */
    if(register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevsettings))
    {
        printk("注册字符设备失败！\r\n");
    }
    else
    {
        printk("模块已加载！\r\n");
    }
    return 0;
}
//驱动出口函数，注销字符设备
static void  __exit chrdev_exit(void)
{
    unregister_chrdev(CHRDEVBASE_MAJOR,CHRDEVBASE_NAME);
    printk("模块已卸载！\r\n");
}

//将函数指定为入口和出口函数（包含注册与注销）
module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("myx");