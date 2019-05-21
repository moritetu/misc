CC = gcc
CFLAGS = -Wall -O2
SRCS = $(wildcard *.c)
PROGRAMS = $(patsubst %.c, %, $(SRCS))

all: $(PROGRAMS)

.PHONY: clean

clean:
	$(RM) $(PROGRAMS)
