CPP = clang++
CPPFLAGS = -Wall -O3 -Isrc $(shell llvm-config --cxxflags --ldflags --system-libs --libs core)

HPP = $(shell find src -name "*.hpp")
SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:src/%.cpp=build/%.o)

lang: $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@
	@chmod +x $@

clean:
	$(RM) -r build/*

build/%.o: src/%.cpp $(HPP)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@
