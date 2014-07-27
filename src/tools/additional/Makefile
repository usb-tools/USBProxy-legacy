
all:
	#gcc -g -I../../Plugins/Devices/ -c dot11_control.c
	#gcc -g -I../../Plugins/Devices/ -c dot11_test.c
	#gcc -g -o dot11_test dot11_test.o dot11_control.o -lusb-1.0
	gcc -g -I../../Plugins/Devices/ dot11_control.c dot11_test.c -o dot11_test -lusb-1.0

clean:
	rm -f dot11_control.o dot11_test.o dot11_test

