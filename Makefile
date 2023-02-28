CC=gcc
CCOPTS=-Wall -Wextra
LIBS=-lm

SRCS=$(wildcard *.c)
TARGETS=$(SRCS:.c=)

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

%: %.c
	$(CC) $(CCOPTS) -o $@ $< $(LIBS)