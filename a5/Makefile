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
all : client server t_server

client : client.c
	$(CC) $(CFLAGS) $< -o $@ 
server : server.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -pthread

t_server : t_server.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -pthread




