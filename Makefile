CC=gcc
CFLAGS=-Wall -Wextra -Iinclude

SRC=$(wildcard src/*.c)
OUTPUT=main

build:
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SRC)

clean:
	rm $(OUTPUT)
