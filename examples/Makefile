include ../Makefile.inc

SRCS = $(wildcard ex_*.c)
BINS = $(patsubst %.c, %, $(SRCS))
HDRS = $(wildcard ../*.h)

.PHONY: all
all: $(BINS) list_test

%: %.c
	$(CC) $(CFLAGS) -o ../bin/$@ $< 

list_test: list_test.c ../ps_list.o
	$(CC) $(CFLAGS) -o ../bin/list_test ../ps_list.o list_test.c



