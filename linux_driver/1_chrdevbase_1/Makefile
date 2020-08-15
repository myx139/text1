#内核路径
kernel_DIR:=/home/myx/linux/IMX6ULL/linux/myx_linux/linux-imx-rel_imx_4.1.15_2.1.0_ga
#当前路径
now_DIR:=$(shell pwd)
#将chrdev.c 编译为模块
obj-m := chrdev.o


build:
	$(MAKE) -C $(kernel_DIR) M=$(now_DIR) modules
APP:
	$(shell arm-linux-gnueabihf-gcc chrdevAPP.c -o chrdevAPP)
clean:
	$(MAKE) -C $(kernel_DIR) M=$(now_DIR) clean
cpmod:
	$(shell sudo cp chrdev.ko /home/myx/linux/nfs/rootfs/lib/modules/4.1.15 -f)
cpAPP:
	$(shell sudo cp chrdevAPP /home/myx/linux/nfs/rootfs/lib/modules/4.1.15 -f)
