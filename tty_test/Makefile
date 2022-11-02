#CROSS=/home/bh92032/IMX7DL_Sabre/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
CC=$(CROSS)gcc
STRIP=$(CROSS)strip
#CFLAGS=-I/home/bh92032/IMX7DL_Sabre/linux-imx-rel_imx_4.1.15_1.2.0_ga/include -I./include
CFLAGS=-I./include 
TARGET=/usr/local/bin

INITRD_DIR :=
INCLUDE_DIR := $(shell pwd)/include
LDFLAGS += -L$(INITRD_DIR)/usr/lib -L$(INITRD_DIR)/lib

GPS_PROG=tty_gps
TEMP_PROG=tty_temp

GPS_OBJS=gps_main.o
TEMP_OBJS=temp_main.o

all: $(GPS_PROG) $(TEMP_PROG) 

tty_gps : $(GPS_OBJS) $(EXTLIB) 
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -lrt -ldl -lm -lcurl -ljansson -o $@
	$(STRIP) $@
	chmod 755 $(GPS_PROG)
	cp $(GPS_PROG) $(TARGET)

tty_temp : $(TEMP_OBJS) $(EXTLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $^  -lrt -ldl -ljansson -lm -lcurl -o $@
	$(STRIP) $@
	chmod 755 $(TEMP_PROG)
	cp $(TEMP_PROG) $(TARGET)

clean:
	rm -f $(TEMP_PROG) $(GPS_PROG)
	rm -f $(OBJS)
	rm -f *.[oa] *.elf *.gdb
	rm -f core
	rm -f $(TARGET)/$(PROG)
