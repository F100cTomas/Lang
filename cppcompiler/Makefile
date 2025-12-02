# Options: debug and release
BUILD ?= debug
ifeq ($(filter $(BUILD),debug release),)
$(error Unsupported BUILD option: $(BUILD))
endif
# Options: linux and mingw
# Defaults to the current one
ifneq ($(OS),Windows_NT)
TARGET ?= linux
else
TARGET ?= mingw
endif
ifeq ($(filter $(TARGET),linux mingw),)
$(error Unsupported TARGET: $(TARGET))
endif
# Main options: clang++ and g++
# If other compiler is supplied it is no problem
COMPILER ?= clang++
CPP := $(COMPILER)
ifneq ($(and $(filter $(COMPILER),g++), $(filter $(TARGET),mingw)),)
	CPP := x86_64-w64-mingw32-g++
endif
# Arguments passed to the compiler
CPPFLAGS_BASE := -Isrc $(shell llvm-config --cxxflags)
ifeq ($(BUILD),debug)
CPPFLAGS := -Wall -g3 -O0 -DDEBUG -fno-omit-frame-pointer $(CPPFLAGS_BASE)
else ifeq ($(BUILD),release)
CPPFLAGS := -Wall -O3 -DNDEBUG $(CPPFLAGS_BASE)
endif
# Arguments passed to the linker
LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core) -llldCommon -llldELF
# Files to be compiled
HPP := $(shell find src -name "*.hpp")
SRC := $(shell find src -name "*.cpp")
OBJ := $(SRC:src/%.cpp=.build/%.o)
# kernel32.lib for mingw targets
KERNEL32 :=
ifeq ($(TARGET),mingw)
KERNEL32 := kernel32.lib
ifneq ($(OS),Windows_NT)
DLLTOOL := x86_64-w64-mingw32-dlltool
else
DLLTOOL := dlltool
endif
endif

# Compilation rules

all: lang $(KERNEL32)

clean:
	$(RM) -r .build/*

lang: $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
	@chmod +x $@

.build/%.o: src/%.cpp $(HPP)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

kernel32.lib: kernel32.def
	$(DLLTOOL) -d $< -l $@

.PHONY: all clean
