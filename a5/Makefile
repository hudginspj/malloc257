#
# Makefile for assign2
#

# Make environment
INCLUDES=-I.
CC=gcc
#-c
CFLAGS=-I. -g -Wall $(INCLUDES) 
LINKARGS=-lm
LIBS=-lm


# Productions
all : client server

client : client.c
	$(CC) $(CFLAGS) $< -o $@ 
server : server.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -pthread




