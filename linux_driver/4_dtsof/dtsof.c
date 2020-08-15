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
/*    backlight {
        compatible = "pwm-backlight";
        pwms = <&pwm1 0 5000000>;
        brightness-levels = <0 4 8 16 32 64 128 255>;
        default-brightness-level = <6>;
        status = "okay";
    };
*/
static int __init dtsof_init(void)
{
    int ret = 0;
    struct device_node * bl_node = NULL;
    struct property * com = NULL;
    const char * str;
    u32 elesize = 0;
    u32 val = 0;
    u32 * level;
    //找到backlight结点，通过路径
    bl_node = of_find_node_by_path("/backlight");
    if (bl_node == NULL)
    {
        ret = -EINVAL;
        goto fail_find;
    }
    //读取字符串方式1
    com = of_find_property(bl_node,"compatible",NULL);
    if (com == NULL)
    {
        ret = -EINVAL;
        goto fail_findpro;
    }
    else
    {
        printk("compatible = %s\r\n",(char *)com->value);
    }
    //读取字符串方式2
    ret =  of_property_read_string(bl_node,"status",&str);
    if(ret < 0)
    {
        goto fail_read_str;
    }
    else
    {
        printk("status = %s\r\n",str);
    }
    //读取数字
    ret = of_property_read_u32(bl_node,"default-brightness-level", &val);
    if(ret < 0)
    {
        goto fail_read_u32;
    }
    else
    {
        printk("default-brightness-level = %d\r\n",val);
    }
    //读取多组数字类的属性
    elesize = of_property_count_elems_of_size(bl_node,"brightness-levels",4);
    if(elesize < 0)
    {
        goto fail_read_ele;
    }
    else
    {
        printk("brightness-levels = %d\r\n",elesize);
    } 
    level = kmalloc(elesize * sizeof(u32),GFP_KERNEL);
    if(level != NULL)
    {
        ret = of_property_read_u32_array(bl_node,"brightness-levels",level,elesize);
        if(ret < 0)
        {
            goto fail_read_u32_array;
        }
        else
        {
            printk("brightness-levels val = %d\r\n",*(level+1));//打印第二个元素
            level = NULL;
            kfree(level);
        }
        
    }
    return 0;
fail_read_u32_array:
    kfree(level);
fail_read_ele:
fail_read_u32:
fail_read_str:
fail_findpro:

fail_find:
    return ret;
}
static void __exit dtsof_exit(void)
{

}

module_init(dtsof_init);
module_exit(dtsof_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("myx");



