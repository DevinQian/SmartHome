
SOURCE = $(wildcard *.c)
TARGETS = $(patsubst %.c, %, $(SOURCE))

CC = gcc
#CROSS = arm-linux-gnueabihf-

INC = -I./

all: find

find: find.c
	$(CC) $(INC) -o find find.c -lpthread 

#listen: listen.c
#	$(CROSS)$(CC) $(INC) -o listen listen.c 


clean: 
	rm -rf $(TARGETS)

.PHONY:clean all


