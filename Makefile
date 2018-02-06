obj-m := pphc.o
pphc-objs := main.o cdev.o

default:

pphc.ko: main.c cdev.c
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install: pphc.ko
	-sudo modprobe -r pphc
	sudo cp pphc.ko /lib/modules/$(shell uname -r)
	sudo depmod
	sudo modprobe pphc

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
