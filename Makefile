CC = g++
TARGET = bin/$(PROJECT)

FLAGS= -std=c++17 -pedantic -ggdb

RELEASE_CXXFLAGS=-O3 -std=c++17


.PHONY: all clean
all: 
	$(CC) -o nn $(FLAGS) nn.c

release: nn.c
	$(CC) -o nn $(RELEASE_FLAGS) nn.c

install: release
	cp nn /usr/bin/nn
	chmod 755 /usr/bin/nn

clean:
	@echo cleaning
	@rm -rf nn
