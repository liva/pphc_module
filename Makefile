obj-m := pphc.o
pphc-objs := main.o cdev.o phc.o

default:

pphc.ko: main.c cdev.c phc.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install: pphc.ko
	-sudo modprobe -r pphc
	sudo cp pphc.ko /lib/modules/$(shell uname -r)
	sudo depmod
	sudo modprobe pphc
	-sudo rm /dev/pphc
	major=`cat /proc/devices | grep pphc | cut -d ' ' -f 1`; sudo mknod /dev/pphc c $$major 0


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
