CC = gcc
CXX = g++
CFLAGS += -std=c99 -Wall -Wextra -pedantic -msse2 -mavx -D__SSE2__ -D__AVX__
CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic -msse2 -mavx -D__SSE2__ -D__AVX__

# Set OBJCOPY if not defined by environment:
OBJCOPY ?= objcopy

# Build mode: 0 is debug mode, 1 is release mode. Example: make RELEASE=1
RELEASE = 1
ARCH = 64
OBJS = src/strlen_fast/strlen_fast.o src/strlen_fast/strlen_fast_avx.o src/strlen_fast/benchmark.o
TARGETS = bin/strlen_fast

ifeq ($(RELEASE), 0)
    # Debug mode
    CFLAGS += -g
    CXXFLAGS += -g
else
    # Release mode
    CFLAGS += -O3 -DNDEBUG
    CXXFLAGS += -O3 -DNDEBUG
endif

ifeq ($(ARCH), 32)
    # x86 arch
    CFLAGS += -m32
    CXXFLAGS += -m32
else
    ifeq ($(ARCH), 64)
        # x64 arch
        CFLAGS += -m64
	CXXFLAGS += -m64
    else
    endif
endif

.PHONY: all analyze clean

all: $(TARGETS)

bin/strlen_fast: $(OBJS)
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
	rm -f $(OBJS) $(TARGETS) $(addsuffix .exe, $(TARGETS))
