obj-m += debug_mod.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	@rm -f debug_mod_test

test:
	gcc debug_mod_test.c -o debug_mod_test
