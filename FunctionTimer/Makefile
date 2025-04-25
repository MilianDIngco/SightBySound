# Variables for compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 

TARGET = libfunctiontimer.a

# Object files
OBJ = functiontimer.o

all: $(TARGET)

$(TARGET): $(OBJ)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: 
	rm -rf $(TARGET) $(OBJ)
