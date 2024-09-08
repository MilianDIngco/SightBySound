# Variables for compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 $(shell pkg-config --cflags opencv)
LDFLAGS = $(shell pkg-config --libs opencv)

test: test.cpp
	$(CXX) $(CXXFLAGS) test.cpp -o test $(LDFLAGS)
clean: 
	rm -f test
