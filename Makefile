CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
CXXFLAGS=-Wall -Wextra -std=c++17 -pedantic -ggdb

RELEASE_CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
RELEASE_CXXFLAGS=-Wall -Wextra -O3 -std=c++17

.PHONY: all
all: 
	$(CXX) -o nn $(CXXFLAGS) nn.c

release:
	$(CXX) -o nn $(RELEASE_CXXFLAGS) nn.c

