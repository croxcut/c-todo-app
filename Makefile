# Makefile for todo.c  :D*
# Requires: MinGW / GCC on Windows
# Usage: make        -> build
#        make run    -> build + run
#        make clean  -> remove output

CC      = gcc
TARGET  = todo.exe
SRC     = todo.c

# strict warnings, optimise, link Windows console libs :D*
CFLAGS  = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lkernel32 -luser32

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	del /Q $(TARGET) 2>nul || rm -f $(TARGET)
