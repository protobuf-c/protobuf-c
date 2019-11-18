obj-m += protobuf-c.o
EXTRA_CFLAGS=-I$(PWD)
KDIR?=/lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean