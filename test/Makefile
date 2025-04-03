# Variables for compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 $(shell pkg-config --cflags opencv4)
LDFLAGS = $(shell pkg-config --libs opencv4) -lopenal -pthread

# Define the programs you want to compile
TARGETS = test capture_checker calibrate_camera set_cameras stereo_calibrate

# Define object files for each program
OBJ_test = test.o
OBJ_checker = capture_checker.o
OBJ_calibrate = calibrate_camera.o
OBJ_set = set_cameras.o
OBJ_stereo = stereo_calibrate.o

# Default target (builds all programs)
all: $(TARGETS)

# Rules for building each target
test: $(OBJ_test)
	$(CXX) -o $@ $^ $(LDFLAGS)

checker: $(OBJ_checker)
	$(CXX) -o $@ $^ $(LDFLAGS)

calibrate: $(OBJ_calibrate)
	$(CXX) -o $@ $^ $(LDFLAGS)

set: $(OBJ_set)
	$(CXX) -o $@ $^ $(LDFLAGS)

stereo: $(OBJ_stereo)
	$(CXX) -o $@ $^ $(LDFLAGS)


# Generic rule to compile .cpp files into .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up compiled files
clean:
	rm -rf $(TARGETS) *.o