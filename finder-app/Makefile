ifndef CROSS_COMPILE
	CROSS_COMPILE :=
endif
CC := $(CROSS_COMPILE)gcc
CFLAGS := -g -Wall -Werror -pedantic -std=c99

all: writer

clean:
	rm -f *.o
	rm -f writer

writer: writer.o
	$(CC) $(CFLAGS) -o writer writer.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: clean all
