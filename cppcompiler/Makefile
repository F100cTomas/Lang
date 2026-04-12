# Options: debug and release
BUILD := debug
ifeq ($(filter $(BUILD),debug release),)
$(error Unsupported BUILD option: $(BUILD))
endif
# Options: linux and mingw
# Defaults to the current one
ifneq ($(OS),Windows_NT)
TARGET := linux
else
TARGET := mingw
endif
ifeq ($(filter $(TARGET),linux mingw),)
$(error Unsupported TARGET: $(TARGET))
endif
# Main options: clang++ and g++
# If other compiler is supplied it is no problem
CXX := clang++
ifneq ($(and $(filter $(CXX),g++), $(filter $(TARGET),mingw)),)
	CXX := x86_64-w64-mingw32-g++
endif
# Arguments passed to the compiler
CXXFLAGS_BASE := -Isrc $(shell llvm-config --cxxflags)
ifeq ($(BUILD),debug)
CXXFLAGS := -Wall -g3 -O0 -DDEBUG -fno-omit-frame-pointer $(CXXFLAGS_BASE)
else ifeq ($(BUILD),release)
CXXFLAGS := -Wall -O3 -DNDEBUG $(CXXFLAGS_BASE)
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
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@chmod +x $@

.build/%.o: src/%.cpp $(HPP)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

kernel32.lib: kernel32.def
	$(DLLTOOL) -d $< -l $@

.PHONY: all clean
