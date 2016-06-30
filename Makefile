CC = gcc
CXX = g++
CFLAGS += -std=c99 -O3 -Wall -Wextra -pedantic -DNDEBUG
CXXFLAGS += -std=c++11 -O3 -Wall -Wextra -pedantic -DNDEBUG

# Set OBJCOPY if not defined by environment:
OBJCOPY ?= objcopy

OBJS =

.PHONY: all analyze clean

all: bin/strlen_fast

bin/strlen_fast: src/strlen_fast/strlen_fast.o
	$(CC) $(CFLAGS) -o $@ $^

bin/strlen_fast: src/strlen_fast/benchmark.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# src/strlen_fast.o: $(OBJS)
#	$(LD) --relocatable -o $@ $^
#	$(OBJCOPY) --keep-global-symbols=lib/exports.txt $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

analyze: clean
	scan-build --use-analyzer=`which clang` --status-bugs make

clean:
	rm -f bin/strlen_fast src/strlen_fast/strlen_fast.o src/strlen_fast/benchmark.o $(OBJS)
