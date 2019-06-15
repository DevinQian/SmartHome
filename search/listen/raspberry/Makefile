OURCE = $(wildcard *.c)
TARGETS = listen

CC = gcc

INC = -I../../

all: $(TARGETS)

listen: listen.c
	$(CC) $(INC) -lpthread -o listen listen.c 

clean: 
	rm -rf $(TARGETS)

.PHONY:clean all
		
