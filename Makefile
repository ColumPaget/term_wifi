VERSION=1.6
CC=gcc
LIBS= -lcrypto -lssl -lUseful-5  
FLAGS=-g -O2 -DVERSION=\"$(VERSION)\"

OBJ=common.o net.o netdev.o runcommand.o iw.o wireless_tools.o wpa_supplicant.o wifi.o interactive.o settings.o help.o

all: $(OBJ) 
	$(CC) $(FLAGS)  -oterm_wifi $(OBJ) main.c $(LIBS)

libUseful-4/libUseful.a:
	$(MAKE) -C libUseful-4

common.o: common.h common.c common.h
	$(CC) $(FLAGS)  -c common.c

interactive.o: interactive.h interactive.c common.h
	$(CC) $(FLAGS)  -c interactive.c

net.o: net.h net.c common.h
	$(CC) $(FLAGS)  -c net.c

help.o: help.h help.c common.h
	$(CC) $(FLAGS)  -c help.c

netdev.o: netdev.h netdev.c common.h
	$(CC) $(FLAGS)  -c netdev.c

wifi.o: wifi.h wifi.c common.h
	$(CC) $(FLAGS)  -c wifi.c

iw.o: iw.h iw.c common.h
	$(CC) $(FLAGS)  -c iw.c

wireless_tools.o: wireless_tools.h wireless_tools.c common.h
	$(CC) $(FLAGS)  -c wireless_tools.c

wpa_supplicant.o: wpa_supplicant.h wpa_supplicant.c common.h
	$(CC) $(FLAGS)  -c wpa_supplicant.c

runcommand.o: runcommand.h runcommand.c common.h
	$(CC) $(FLAGS)  -c runcommand.c

settings.o: settings.h settings.c common.h
	$(CC) $(FLAGS)  -c settings.c


clean:
	rm term_wifi *.o libUseful-4/*.o libUseful-4/*.a libUseful-4/*.so

test:
	echo "no tests"
