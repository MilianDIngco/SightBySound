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

SETTING_TARGET = setting_writer

DATA_TARGET = data_reader

# Object files
OBJ = save_wav.o main.o
SETTING_OBJ = setting_writer.o
DATA_OBJ = data_reader.o

all: $(TARGET)

settings: $(SETTING_TARGET)

data: $(DATA_TARGET)

$(TARGET): $(OBJ) 
	$(CXX) -o $@ $^ $(LIBRARY) $(LDFLAGS) -lopenal -pthread

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o 

setting_writer.o: setting_writer.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

data_reader.o: data_reader.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SETTING_TARGET): $(SETTING_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(DATA_TARGET): $(DATA_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@

clean: 
	rm -rf $(TARGET) $(OBJ) $(SETTING_TARGET) $(SETTING_OBJ) $(DATA_TARGET) $(DATA_OBJ)
