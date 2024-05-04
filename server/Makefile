CROSS_COMPILE ?=
CC ?= $(CROSS-COMPILE)gcc

CFLAGS ?= -g -Wall -Werror -pedantic -std=c99
LDFLAGS ?= -pthread -lrt
TARGET ?= aesdsocket

all: $(TARGET)

clean:
	rm -rf *.o
	rm -rf aesdsocket

aesdsocket: aesdsocket.o
	$(CC) -o aesdsocket aesdsocket.o $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean all
