COMPILE_FOLDERS := src
TARGET := sandblox
CC := gcc
CFLAGS := -Wall -Wextra -O3 -Iinclude
ifeq ($(OS),Windows_NT)
	LDFLAGS := -lSDL3 -lSDL3_image resource.res
else
	LDFLAGS := -lm -lSDL3 -lSDL3_image
endif

SOURCES := $(wildcard $(addsuffix /*.c,$(COMPILE_FOLDERS)))
OBJECTS := $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
