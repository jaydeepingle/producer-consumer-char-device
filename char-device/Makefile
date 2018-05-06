obj-m += process_list.o

KDIR = /usr/src/linux-headers-$(shell uname -r)
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

insert:
	sudo insmod process_list.ko     

compile:
	gcc app.c

run:
	sudo ./a.out

clean:
	rm -rf .o *.ko *.mod. *.symvers *.order *~

remove:
	sudo rmmod process_list
