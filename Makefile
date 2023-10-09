CC=gcc
CFLAGS=-Wall -Wextra -Iinclude

SRC=$(wildcard src/*.c)
OUTPUT=main

build:
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SRC)

test: build
	bundle exec rspec test/test.rb

clean:
	rm $(OUTPUT)
