CXX = emcc
CXXFLAGS = -lembind -s LLD_REPORT_UNDEFINED

SRC = src/*.cpp
INCLUDE = -Isrc/include

all: wasm

wasm:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC) main.cpp -o build/index.html

clean: 
	rm -rf build/index*