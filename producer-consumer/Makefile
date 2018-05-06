obj-m += linepipe.o

KDIR = /usr/src/linux-headers-$(shell uname -r)
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

insert:
	sudo insmod linepipe.ko n=3

compile:
	gcc producer.c -o pro
	gcc consumer.c -o con

produce:
	sudo ./pro /dev/linepipe

consume:
	sudo ./con /dev/linepipe

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *~

remove:
	sudo rmmod linepipe
