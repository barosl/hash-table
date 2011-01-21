.PHONY: clean all

CFLAGS = -Wall -Werror -O3

STRIP = strip

all: hash-table-mem hash-table-disk

hash-table-mem: main-mem.o htable.o
	$(CC) -o $@ $+
	$(STRIP) $@

hash-table-disk: main-disk.o htable.o clock.o
	$(CC) -o $@ $+
	$(STRIP) $@

clean:
	rm -f hash-table-mem hash-table-disk *.o
