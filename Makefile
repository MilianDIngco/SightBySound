# Variables for compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 $(shell pkg-config --cflags opencv4) -Wno-deprecated-enum-enum-conversion -O2
LDFLAGS = $(shell pkg-config --libs opencv4)
DEBUG =? 0

LIBRARY=FunctionTimer/libfunctiontimer.a

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
else
	CXXFLAGS += -DNDEBUG
endif

TARGET = main

# Object files
OBJ = save_wav.o main.o

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CXX) -o $@ $^ $(LIBRARY) $(LDFLAGS) -lopenal -pthread

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o 

%.o: %.cpp
	$(CXX) -c $^ -o $@

clean: 
	rm -rf $(TARGET) $(OBJ)
