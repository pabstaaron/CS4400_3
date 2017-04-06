#
# Makefile for the malloc lab driver
#
CC = gcc
CFLAGS = -O2 -Wall

OBJS = mdriver.o mm.o memlib.o pagemap.o fsecs.o fcyc.o clock.o ftimer.o
MOBJS = myDriver.c mm.o memlib.o pagemap.o #fsecs.o fcyc.o clock.o ftimer.o

all: mdriver mydriver

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS) -lm

mydriver: $(MOBJS)
	$(CC) -o mydriver $(MOBJS) 

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
mydriver.o: myDriver.c 
memlib.o: memlib.c memlib.h pagemap.h
pagemap.o: pagemap.c pagemap.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

clean:
	rm -f *~ *.o mdriver mydriver
