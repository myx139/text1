#include <linux/printk.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h> 
#include <linux/module.h>
#include <linux/ide.h>

#define chrdevbase_major 200
#define chrdevbase_name  "myx"

static char readbuffer[100];
static char writebuffer[100];
static char data[] = {"Hello I'm MYX'Linux!"};


static int chrdevbase_open(struct inode *inode, struct file *filp)
{
   // printk("chardevbase is open\r\n");
    return 0;
}
static int chrdevbase_release(struct inode *inode,struct file *filp)
{
   // printk("chardevbase is release\r\n");
    return 0;
}
static ssize_t chrdevbase_read(struct file *filp, char __user *buf,size_t cnt, loff_t *offt)
{
    int ret = 0;
    //printk("chardevbase is reading\r\n");
    memcpy(readbuffer,data,sizeof(data));
    ret  = copy_to_user(buf,readbuffer,cnt);
    if(ret == 0)
    {

    }
    else
    {
        /* code */
    }
    
    return 0;
}
static ssize_t chrdevbase_write(struct file *filp,const char __user *buf,size_t cnt, loff_t *offt)
{
    int ret = 0;
    ret = copy_from_user(writebuffer,buf,cnt);
    if(ret == 0)
    {
        printk("recevdata :%s\r\n",writebuffer);
    }
    else
    {
        /* code */
    }
    
    return 0;
}
/*字符设备操作集合*/
static struct file_operations chrdevbase_fops={
    .owner = THIS_MODULE,
    .open  = chrdevbase_open,
    .release = chrdevbase_release,
    .read  = chrdevbase_read,
    .write = chrdevbase_write,
};
static int __init chrdevbase_init(void)
{
    int ret = 0;
    ret = register_chrdev(chrdevbase_major,chrdevbase_name,
                        &chrdevbase_fops);
    if(ret < 0)     
    printk("chrdevbase_init is failed\r\n");
    printk("模块已加载\r\n");
	return 0;
}

static void __exit chrdevbase_exit(void)
{
    unregister_chrdev(chrdevbase_major,chrdevbase_name);
    printk("模块已卸载\r\n");
}

/*
    模块入口与出口
*/
module_init(chrdevbase_init);//模块加载函数
module_exit(chrdevbase_exit);//模块卸载函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



