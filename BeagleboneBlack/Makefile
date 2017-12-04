CC     = gcc

CFLAGS = -g -std=c99 #-Werror -Wall

IPATH  = -I./inc

LIBS   =  -pthread -lrt

FILES  =  src/main.c
FILES  += src/logger.c
FILES  += src/common.c
FILES  += src/my_socket.c

all:
	$(CC) $(CFLAGS) $(IPATH) $(FILES) $(LIBS) -o plant_server
	@echo BBB Server Build Complete!

.PHONY: clean

clean:
	@rm -vf plant_server
	@echo Server Executable Removed!
