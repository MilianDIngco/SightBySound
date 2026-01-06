# SightBySound
## Depth Map Sonification

This project generates audio from grayscale depth maps using a Hilbert curve traversal. Each pixel is associated with a frequency based on its position, and the pixel's brightness modulates the volume of the frequency. The final audio output is a composite of all the mapped frequencies, allowing the depth information to be perceived through sound.

## Features

- Depth-to-audio conversion via Hilbert curve
- Debug and timing modes for performance testing
- Customizable experiments for different settings
- Data collection and analysis tools

## Getting Started

### Prerequisites

- A C++ compiler (e.g., `g++`)
- `make`

### Compiling and Running

#### Run the main program:
```bash
make && ./main
```

### Run in Debug mode (with timing information)
```bash
make DEBUG=1 && ./main
```

### Run tests for different settings and see results
```bash
make data && make settings && make DEBUG=1 && ./settings [VALID_VARIABLE_NAME] [NUMBER_OF_RUNS] && ./data_reader
```

Run make clean if a new ./main file was not generated after running make DEBUG=1

Ensure camera dynamic link refers to the correct path on different computers

# Repository Structure
- main.cpp – The entry point for generating and playing audio.

- settings.txt - Where settings are edited before runtime.

- settings.cpp – Used to run experiments with different parameter values.

- data_reader.cpp – Parses and outputs results from experiments.

- Makefile – Build automation.

- README.md – Project overview and usage instructions.

- test folder - Camera calibration scripts

## To Do:
- Clean up and add documentation to camera calibration scripts, rename test folder
- Move FunctionTimer static library to separate repository

# Citation

M. Ingco and S. Yoon, “Sight By Sound: Real-Time Sonification of Stereo Depth Maps using Hilbert Curves for Assistive Navigation supported by a Virtual Training Environment,” in proceedings of 2026 IEEE International Conference on Artificial Intelligence and eXtended and Virtual Reality (AIxVR), Osaka, Japan, January 26-28, 2026.
