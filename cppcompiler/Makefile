ifeq ($(OS),Windows_NT)
CPP = clang++
CPPFLAGS = -Wall -O3 -Isrc $(shell llvm-config --cxxflags)
LDFLAGS = $(shell llvm-config --ldflags --system-libs --system-libs --libs all)

HPP = $(shell find src -name "*.hpp")
SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:src/%.cpp=.build/%.o)

all: lang kernel32.lib

clean:
	rm -rf .build/*

lang: $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
	@chmod +x $@

.build/%.o: src/%.cpp $(HPP)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

kernel32.lib: kernel32.def
	dlltool -d $< -l $@

.PHONY: all clean
else
CPP = clang++
CPPFLAGS = -Wall -O3 -Isrc $(shell llvm-config --cxxflags)
LDFLAGS = $(shell llvm-config --ldflags --system-libs --libs core) -llldCommon -llldELF

HPP = $(shell find src -name "*.hpp")
SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:src/%.cpp=.build/%.o)

all: lang

clean:
	$(RM) -r .build/*

lang: $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
	@chmod +x $@

.build/%.o: src/%.cpp $(HPP)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

.PHONY: all clean
endif