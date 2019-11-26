#
# Makefile for the UM Test lab
# 
CC = gcc

IFLAGS  = -I/comp/40/bin/include -I/usr/sup/cii40/include/cii -I/comp/40/build/include 
CFLAGS  = -O2 -g -std=gnu99 -Wall -Wextra -Werror -pedantic $(IFLAGS)
LDFLAGS = -g -L/comp/40/bin/lib -L/usr/sup/cii40/lib64 -L/comp/40/build/lib
LDLIBS  = -lcii40-O2 -l40locality -lcii40 -lm -lbitpack

EXECS   = um

all: $(EXECS)

um: um.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECS)  *.o

