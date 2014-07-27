
all:
	gcc -I../../Plugins/Devices/ -c dot11_control.c
	gcc -I../../Plugins/Devices/ -c dot11_test.c
	gcc -o dot11_test dot11_test.o dot11_control.o -lusb-1.0

clean:
	rm -f dot11_control.o dot11_test.o dot11_test

