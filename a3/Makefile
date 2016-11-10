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

# Files
OBJECT_FILES=	assign2.o \
				malloc.o

# Productions
all : main multi

main : main.c
	$(CC) $(CFLAGS) $< -o $@ 
multi : multithreaded.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -pthread




