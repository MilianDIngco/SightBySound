# SightBySound

![Project Logo](media/capture.jpg)

Depth Map Sonification: This project generates audio from grayscale depth maps using a Hilbert curve traversal. Each pixel is associated with a frequency based on its position, and the pixel's brightness modulates the volume of the frequency. The final audio output is a composite of all the mapped frequencies, allowing the depth information to be perceived through sound.

## :loudspeaker: News
- January 7, 2026: SightBySound project is live! :tada:

## :sparkles: Features

- Depth-to-audio conversion via Hilbert curve
- Debug and timing modes for performance testing
- Customizable experiments for different settings
- Data collection and analysis tools

## :rocket: Getting Started

### :package: Dependencies

- [OpenCV 4+](https://opencv.org)
- [OpenAL](https://openal.org) or [OpenAL Soft](https://openal-soft.org/)

### :clipboard: Prerequisites

- Two statically mounted cameras
- A speaker
- A C++ compiler (e.g., `g++`)
- `make`

### :hammer_and_wrench: Compiling and Running

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

## :open_file_folder: Repository Structure

```text
.
├── 📂 gtests/             # Scripts for camera calibration
├── 📂 include/            # Header files
├── 📂 src/                # Source files
│   └── 📄 main.cpp        # The entry point for generating and playing audio.
│   └── 📄 settings.cpp    # Used to run experiments with different parameter values.
|   ...
├── 📂 settings/           # Settings are edited here before runtime.
├── 📄 .gitignore          # Git ignore rules
├── 📄 CMakeList.txt       # CMakeList
└── 📄 README.md           # Project overview and usage instructions.
```

## :date: To Do:

<dl>
  <dt></dt>
  <dd><span aria-hidden="true">:hourglass_flowing_sand:</span> Clean up and add documentation to camera calibration scripts, rename test folder </dd>
  
  <dt></dt>
  <dd><span aria-hidden="true">:hourglass_flowing_sand:</span> Move FunctionTimer static library to separate repository</dd>
</dl>


## :scroll: Citation

```text
@inproceedings{Ingco2026Sight,
  author    = {Ingco, M. and Yoon, S.},
  title     = {Sight By Sound: Real-Time Sonification of Stereo Depth Maps using Hilbert Curves for Assistive Navigation supported by a Virtual Training Environment},
  booktitle = {Proceedings of the 2026 IEEE International Conference on Artificial Intelligence and eXtended and Virtual Reality (AIxVR)},
  year      = {2026},
  month     = {Jan},
  address   = {Osaka, Japan},
  note      = {January 26--28},
  publisher = {IEEE}
}
```


