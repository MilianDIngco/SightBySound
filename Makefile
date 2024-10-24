# Variables for compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 $(shell pkg-config --cflags opencv4)
LDFLAGS = $(shell pkg-config --libs opencv4)


test: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main $(LDFLAGS) -lopenal
clean: 
	rm -f main
