#makefile for music xml - inspiration from CS50 Pset 6 - Huffman Encoding

#compiler
CC = clang

#compiler flags
CFLAGS = -ggdb -O0 -Qunused-arguments -std=c99 -Wall -Werror

#import
IMPORT = import
IMPORT_SRCS = import.c musicxml.c
IMPORT_OBJS = $(IMPORT_SRCS:.c=.o)

#headers
HDRS = musicxml.h

#libraries
XML_LIBS = -I/usr/include -lm -lxml2
GSL_LIBS = -I/usr/local/include -L/usr/local/lib -lm -lgsl

$(IMPORT): $(IMPORT_OBJS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(IMPORT_OBJS) $(XML_LIBS) $(GSL_LIBS)

clean:
	rm -f core $(LIBTEST) *.o
