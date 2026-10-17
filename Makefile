CC ?= gcc
CFLAGS ?= -O2 -Wall

xkbfix.so: xkbfix.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $< -ldl

clean:
	rm -f xkbfix.so

.PHONY: clean
