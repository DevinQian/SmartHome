
SOURCE = $(wildcard *.c)
TARGETS = $(patsubst %.c, %, $(SOURCE))

CC = gcc
#CROSS = arm-linux-gnueabihf-


all: find listen

find: find.c
	$(CC) -o find find.c -lpthread

listen: listen.c
	$(CROSS)$(CC) -o listen listen.c


clean: 
	rm -rf $(TARGETS)

.PHONY:clean all


