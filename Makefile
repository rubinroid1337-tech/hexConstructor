CC      := gcc
CFLAGS  := -std=c23 -O2 -Wall -Wextra -Wpedantic
LDFLAGS := -lraylib -lm

TARGET  := hexConstructor
SRCS    := $(wildcard src/*.c src/core/*.c src/input/*.c src/render/*.c)
OBJS    := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean

