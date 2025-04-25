# Variables for compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 $(shell pkg-config --cflags opencv4) -Wno-deprecated-enum-enum-conversion
LDFLAGS = $(shell pkg-config --libs opencv4)
DEBUG =? 0

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
endif

TARGET = main

# Object files
OBJ = save_wav.o main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS) -lopenal -pthread

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o 

%.o: %.cpp
	$(CXX) -c $^ -o $@

clean: 
	rm -rf $(TARGET) $(OBJ)
