CPP = clang++
CPPFLAGS = -Wall -O3 -std=c++20 -Isrc $(shell llvm-config --cxxflags --ldflags --system-libs --libs core)

all: ferro

clean:
	$(RM) -r cache/*

SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:src/%.cpp=cache/%.o)
ferro: $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@
	@chmod +x $@

cache/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@
