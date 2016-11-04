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
multi : multi_block.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -pthread

#assign2 : $(OBJECT_FILES)
#	$(CC) $(LINKARGS) $(OBJECT_FILES) -o $@ $(LIBS)

#assign2.o : assign2.c malloc.h
#	$(CC) $(CFLAGS) $< -o $@

#malloc.o : malloc.c malloc.h 
#	$(CC) $(CFLAGS) $< -o $@

#clean : 
#	rm -f assign2 $(OBJECT_FILES)
