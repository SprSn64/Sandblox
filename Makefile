DIRS := src src/studio src/cjosn

CC  := gcc
CXX := g++
CFLAGS   := -Wall -Wextra -O3 -Iinclude
CXXFLAGS := $(CFLAGS) -std=c++17

ifeq ($(OS),Windows_NT)
	TARGET := sandblox
	LDFLAGS := -lSDL3 -lSDL3_image -lgdi32 -lopengl32 -lglew32 resource.res
else
	TARGET := sandblox.$(shell uname -m)
	LDFLAGS := -lm -lSDL3 -lSDL3_image -lGL -lGLEW
endif

C_SOURCES   := $(wildcard $(addsuffix /*.c,$(DIRS)))
CPP_SOURCES := $(wildcard $(addsuffix /*.cpp,$(DIRS)))

OBJECTS := \
	$(C_SOURCES:.c=.o) \
	$(CPP_SOURCES:.cpp=.o)

LINKER := $(CXX)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
