#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <sstream>  
#include <AL/al.h>
#include <AL/alc.h>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <variant>
#include "save_wav.h"
#include <vector>
#include <time.h>

/*
Naming Conventions
variables are named using snake_case
global variables are ALL_CAPS
methods use camelCase
*/

using ValueType = std::variant<int, double, std::string, bool>;

// Setting variables
int ORDER = 3; // order of hilbert curve, dimensions are determined from this
double MIN_FREQ = 100; // lowest frequency
double MAX_FREQ = 500; // highest frequency
double DURATION = 5; // duration in seconds
int SAMPLE_RATE = 22000; // number of samples per second
std::string IMAGE_URL = "white.png"; // url to test image
double VOLUME = 1; // 0 - 1
bool PLAY_AUDIO = 1;
bool CAP_FROM_CAMERA = 1;
bool SAVE_IMG = 0;
int CAM_1_INDEX = 0;
int CAM_2_INDEX = 1;
int temp_N_TESTS = 1;
bool temp_DO_IMG_TEST = 0;
bool temp_DO_SOUND_TEST = 0;

struct Pair {
    int x;
    int y;

    Pair() : x(0), y(0  ) {}
    Pair(int x, int y) : x(x), y(y) {}
    Pair(const Pair& p) : x(p.x), y(p.y) {}

    std::string toString() {
      std::stringstream res;
      res << "(" << x << ", " << y << ")";
      return res.str(); 
    }
};

/** Param: int index
    Return: Pair
    Takes an index from 0 - 2^(2 * ORDER) of the hilbert curve,
    and returns the resulting coordinates as a Pair. 
 */
Pair getCoord(int index) {
    // bounds checking; Number of points is ( 2 ^ order ) ^ 2 since it's a square
    // array for now
    if (index >= std::pow(4, ORDER))
      return Pair(-1, -1);

    Pair order1[] = {
      Pair(0, 0), Pair(0, 1),
      Pair(1, 1), Pair(1, 0)
    };

    // Depth of recursion is mapped by (log base 4) + 1
    // Index < 4 : order 1
    // Index < 16 : order 2 ...
    int depth = ORDER;

    int quadrant = index & 3;
    Pair coord(order1[quadrant]);
    // Start at order 1
    for (int i = 1; i < depth; i++) {
      // Gets if in quadrant 0, 1, 2, or 3
      index >>= 2;
      int nextq = index & 3;

      // First quadrant ? Reflect across y = x
      // Multiply by matrix
      // [ 0 1 ]
      // [ 1 0 ]
      // Equivalent to swapping x and y
      if (nextq == 0) {
        int temp = coord.y;
        coord.y = coord.x;
        coord.x = temp;
        // Fourth quadrant ?
      } else if (nextq == 3) {
        // Translate to center the origin
        double dist = (std::pow(2, i) - 1) / 2.0; // dist is half the width of a quadrant
        double xTranslated = ((double) coord.x) - dist;
        double yTranslated = ((double) coord.y) - dist;
        double temp = yTranslated;
        // Swap and take the negatives
        yTranslated = -xTranslated;
        xTranslated = -temp;
        // Translate back to regular
        coord.x = (int) (xTranslated + dist);
        coord.y = (int) (yTranslated + dist);
      }

      // Right quadrants ?
      // Add 2^order to x
      // Moves coordinate over by half of the width of the new square
      if (nextq == 2 || nextq == 3) {
        coord.x += std::pow(2, i);
      }

      // Bottom quadrants ?
      // Add 2^order to y
      // Moves coordinate over by half of the width of the new square
      if (nextq == 1 || nextq == 2) {
        coord.y += std::pow(2, i);
      }
    }
    return coord;
}

/** Param:  short*& samples - Empty array to be filled with audio samples
            int sample_rate - # of samples / second
            float duration  - Duration of time the generated audio clip will be
    Return: int
    Takes a pointer to an empty array of type short. 
 */
int genSampleArray(short*& samples, int sample_rate, float duration) {
    int sample_count = static_cast<int>(sample_rate * duration);

    try {
        samples = new short[sample_count];
    } catch (std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return 1;
    }

    return sample_count;
}

void generateSines(short* samples, int sample_count, int n_pixel, int sample_rate, float* volumes, float* freqs) {
  // std::cout << sample_count << " " << n_pixel << " " << sample_count * n_pixel << std::endl;
  for (int i = 0; i < sample_count; i++) {
    float sample = 0;
    for (int n = 0; n < n_pixel; n++) {
        sample += std::sin(2.0f * M_PI * freqs[n] * static_cast<float>(i) / sample_rate) * volumes[n];
    }
    samples[i] = static_cast<short>((sample / n_pixel) * 32767 );
    // samples[i] = static_cast<short>(sample * 32767);
  } 
}

void generateHilbert(struct Pair hilbert[], int n_pixels) {
  for (int i = 0; i < n_pixels; i++) {
    hilbert[i] = getCoord(i);
  }
}

void generateFrequencies(float freqs[], double min_freq, double max_freq, int n_pixels) {
  double freq_step = (max_freq - min_freq) / n_pixels;
  for (int i = 0; i < n_pixels; i++) {
    freqs[i] = min_freq + (i * freq_step);
  }
}

void generateVolumes(float volumes[], cv::Mat image, struct Pair hilbert[], int n_pixels) {
  for (int i = 0; i < n_pixels; i++) {
    float pixel_value = static_cast<float>(image.at<uchar>(hilbert[i].x, hilbert[i].y));
    volumes[i] = pixel_value / 255.0f;
  }
}

void captureImage(cv::Mat& image, cv::VideoCapture cap, int image_width) {
  // Grab image
  if (CAP_FROM_CAMERA) 
    cap >> image;
  else
    image = cv::imread(IMAGE_URL);

  // save image
  if (SAVE_IMG) 
    cv::imwrite("in.png", image);

  // scale image down
  cv::Size scaled_size(image_width, image_width);
  cv::resize(image, image, scaled_size, 0, 0, cv::INTER_NEAREST);

  // First, try just using a gray scale image
  cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

  // save image
  if (SAVE_IMG) 
    cv::imwrite("out.png", image);
}

template <typename Func, typename...Args>
void timeFunction(int n_runs, const std::string test_name, Func func, Args... args) {
  clock_t start, end;
  start = clock();
  for (int i = 0; i < n_runs; i++) {
    func(args...);
  }
  end = clock();

  double elapsed_time = double(end - start) / CLOCKS_PER_SEC;
  double avg_time = elapsed_time / n_runs;

  std::cout << test_name << " took " << elapsed_time << " secs to run " << n_runs << " tests.\nAverage time per test: " << avg_time << std::endl; 
}

int main(int argc, char** argv) {

  // ------------------------------------------------Read in settings from text file----------------
    std::ifstream settings ("settings.txt");
    if (settings.is_open()) {
      std::cout << "Settings opened successfully" << std::endl;

      std::unordered_map<std::string, ValueType> setting_map;
      std::string line;
      while (std::getline(settings, line)) {
        std::stringstream s(line);

        // Grab the type
        char type;
        s >> type;
        type = std::tolower(type);

        // Grab the name
        std::string name;
        s >> name;
        
        // Grab the value
        s.ignore(1); // skip space
        std::string value;
        std::getline(s, value);

        switch (type) {
          case 'i':
            setting_map[name] = std::stoi(value);
            break;
          case 'd':
            setting_map[name] = std::stod(value);
            break;
          case 's':
            setting_map[name] = value;
            break;
          case 'b':
            setting_map[name] = (bool) std::stoi(value);
            break;
          default:
            std::cout << "ERROR: Setting type not defined";
            return 1;
        }
      }

      auto getValue = [&setting_map]<typename T>(const std::string& name, T& var) {
        var = std::get<T>(setting_map.at(name));
      };

      getValue("ORDER", ORDER);
      getValue("MIN_FREQ", MIN_FREQ);
      getValue("MAX_FREQ", MAX_FREQ);
      getValue("DURATION", DURATION);
      getValue("SAMPLE_RATE", SAMPLE_RATE);
      getValue("IMAGE_URL", IMAGE_URL);
      getValue("VOLUME", VOLUME);
      getValue("PLAY_AUDIO", PLAY_AUDIO);
      getValue("CAP_FROM_CAMERA", CAP_FROM_CAMERA);
      getValue("SAVE_IMG", SAVE_IMG);
      getValue("CAM_1_INDEX", CAM_1_INDEX);
      getValue("CAM_2_INDEX", CAM_2_INDEX);
      getValue("temp_N_TESTS", temp_N_TESTS);
      getValue("temp_DO_IMG_TEST", temp_DO_IMG_TEST);
      getValue("temp_DO_SOUND_TEST", temp_DO_SOUND_TEST);

    } else {
      std::cout << "Settings file failed to open\n Default settings applied" << std::endl;
      // leave default settings
    }

    settings.close();

  // ------------------------------------------------Derive variables, Open camera----------------   
    int image_width = std::pow(2, ORDER);
    int n_pixels = image_width * image_width;
    cv::VideoCapture cap(CAM_1_INDEX);

    if (!cap.isOpened()) {
      std::cerr << "Error: Could not open webcam" << std::endl;
      return -1;
    }
  // ------------------------------------------------Generate hilbert pair array------------------------
    struct Pair hilbert[n_pixels];
    generateHilbert(hilbert, n_pixels);
    std::cout << "hilbert done" << std::endl;
  // ------------------------------------------------Generate frequency array------------------------------------------------
    float freqs[n_pixels];
    generateFrequencies(freqs, MIN_FREQ, MAX_FREQ, n_pixels);
    std::cout << "freq done" << std::endl;

  // ------------------------------------------------Sine waves------------------------------------------------
    float sine[1]; // needs sample count

    // GOAL: generate an array of samples

    std::cout << "sine waves done" << std::endl;

  // ------------------------------------------------Capture image------------------------------------------------
    // Open jpg file
    cv::Mat image;

    if (temp_N_TESTS > 0 && temp_DO_IMG_TEST) 
      timeFunction(temp_N_TESTS, "Capture Image", captureImage, image, cap, image_width);
    captureImage(image, cap, image_width);
    std::cout << "image done" << std::endl;

  // ------------------------------------------------Volume array------------------------------------------------
    float volumes[n_pixels];
    generateVolumes(volumes, image, hilbert, n_pixels);
    std::cout << "volume done" << std::endl;

  // Generate Sine Wave and load into buffer
    short* samples = nullptr;
    int sample_count = genSampleArray(samples, SAMPLE_RATE, DURATION);
    if (temp_N_TESTS > 0 && temp_DO_SOUND_TEST)
      timeFunction(temp_N_TESTS, "Generate samples", generateSines, samples, sample_count, n_pixels, SAMPLE_RATE, volumes, freqs);
    else
      generateSines(samples, sample_count, n_pixels, SAMPLE_RATE, volumes, freqs);
    std::cout << "samples done" << std::endl;

  if (!PLAY_AUDIO) {
    std::vector<short> sample_v(samples, samples + sample_count);
    saveWav("sound.wav", sample_v, SAMPLE_RATE, 1);
  }

  if (PLAY_AUDIO) {
    // Initialize OpenAL and create a context
      ALCdevice *device = alcOpenDevice(nullptr); // open default device
      if (!device) {
          std::cerr << "Error: Could not open sound device." << std::endl;
          return -1;
      }

      std::cout << "open al device done" << std::endl;

      ALCcontext *context = alcCreateContext(device, nullptr);
      if (!context || !alcMakeContextCurrent(context)) {
          std::cerr << "Error: Could not create or set context." << std::endl;
          if (context) alcDestroyContext(context);
          alcCloseDevice(device);
          return -1;
      }
    std::cout << "create al context done" << std::endl;
    
    // Create a buffer and fill it with the generated sine wave data
    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, samples, sample_count * sizeof(short), SAMPLE_RATE);
    std::cout << "al buffers done" << std::endl;

    // Create a source to play the buffer
    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    std::cout << "PLAYING" << std::endl;
    // Play the sound
    alSourcePlay(source);

    // Wait for the sound to finish playing
    ALint source_state;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    while (source_state == AL_PLAYING) {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    }
    std::cout << "AUDIO FINISHED " << std::endl;

    // Clean up OpenAL resources
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    std::cout << "Resources deleted" << std::endl;

    // Close OpenAL context and device
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    std::cout << "al context closed" << std::endl;
  }
  
  // Delete the generated sine wave data
  delete[] samples;
  std::cout << "samples deleted done" << std::endl;

  return 0;
}