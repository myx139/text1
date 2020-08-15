#/*内核路径*/
KERNELDIR:=/home/myx/linux/IMX6ULL/linux/myx_linux/linux-imx-rel_imx_4.1.15_2.1.0_ga
#/*当前路径*/
CURRENT_PATH:= $(shell pwd)
#/*编译完的文件*/
obj-m := dtsled.o
modules_name := dtsled.ko
#APP_name := ledAPP
#/*表示编译的是模块*/
build: kernel_modules
kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
cpmod:
	$(shell sudo cp $(modules_name) /home/myx/linux/nfs/rootfs/lib/modules/4.1.15/ -f)
cpAPP:
	$(shell sudo cp $(APP_name) /home/myx/linux/nfs/rootfs/lib/modules/4.1.15/ -f)