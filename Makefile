# ------------------------------------------------------
# Makefile for bongcloud, made by the larrabyte himself.
# ------------------------------------------------------
SRCFILES := $(wildcard src/*.cpp)
OBJFILES := $(SRCFILES:src/%.cpp=obj/%.o)
DEPFILES := $(SRCFILES:src/%.cpp=obj/%.d)

WARNINGS  := -Wall -Wextra -Wpedantic
INCLUDES  := -Iinclude -Icenturion/src -Iargparse/include $(shell sdl2-config --cflags)
LIBRARIES := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lfmt

CFLAGS := $(WARNINGS) $(INCLUDES) -MD -MP -std=c++20 -O3 -flto -DNDEBUG
LFLAGS := $(LIBRARIES)

all: bongcloud
-include $(DEPFILES)

bongcloud: $(OBJFILES)
	@$(CXX) $(CFLAGS) $(OBJFILES) -o bin/bongcloud $(LFLAGS)
	@printf "[linking] binary created.\n"
	@./bin/bongcloud -b

clean:
	@rm -f obj/*
	@printf "[cleaner] removed object files.\n"
	@rm -f bin/*
	@printf "[cleaner] removed built binaries.\n"

obj/%.o: src/%.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@
	@printf "[cpp2obj] $< compiled.\n"
