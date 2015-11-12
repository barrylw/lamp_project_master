<<<<<<< HEAD
CONTIKI=../..

CFLAGS=-Wall -Werror -I$(CONTIKI)/core -I.

all: wpcapslip

vpath %.c $(CONTIKI)/core/net

wpcapslip: wpcapslip.o wpcap.o tcpdump.o

%: %.o
	$(CC) $(LDFLAGS) $^ /lib/w32api/libws2_32.a /lib/w32api/libiphlpapi.a -o $@
=======
CONTIKI=../..

CFLAGS=-Wall -Werror -I$(CONTIKI)/core -I.

all: wpcapslip

vpath %.c $(CONTIKI)/core/net

wpcapslip: wpcapslip.o wpcap.o tcpdump.o

%: %.o
	$(CC) $(LDFLAGS) $^ /lib/w32api/libws2_32.a /lib/w32api/libiphlpapi.a -o $@
>>>>>>> 1c8a365a388f3826ae65a4404b1caaf07e71bb24
