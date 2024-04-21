ifndef CROSS_COMPILE
	CROSS_COMPILE :=
endif

ifndef CC
	CC := $(CROSS-COMPILE)gcc
endif

CFLAGS += -g -Wall -Werror -pedantic -std=c99

all: aesdsocket

clean:
	rm -rf *.o
	rm -rf aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(LDFLAGS) -o aesdsocket aesdsocket.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean all
