# --------------------------------------------------
# Makefile for chess, made by the larrabyte himself.
# --------------------------------------------------
SRCFILES := $(wildcard src/*.cpp)
OBJFILES := $(SRCFILES:src/%.cpp=obj/%.o)
DEPFILES := $(SRCFILES:src/%.cpp=obj/%.d)

WARNINGS := -Wall -Wextra -Wpedantic
INCLUDES := -Iinclude
SDL2INC  := $(shell sdl2-config --cflags)
SDL2LIB  := $(shell sdl2-config --libs)

CFLAGS := $(WARNINGS) $(INCLUDES) $(SDL2INC) -MD -MP -std=c++20 -O2
LFLAGS := $(SDL2LIB)

all: chess
-include $(DEPFILES)

chess: $(OBJFILES)
	@$(CXX) $(CFLAGS) $(OBJFILES) -o bin/chess $(LFLAGS)
	@printf "[linking] binary created.\n"
	@./bin/chess

clean:
	@rm -f obj/*
	@printf "[cleaner] removed object files.\n"
	@rm -f bin/*
	@printf "[cleaner] removed built binaries.\n"

obj/%.o: src/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@printf "[cpp2obj] $< compiled.\n"
