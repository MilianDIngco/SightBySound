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
#include <time.h> // TODO: replace timing function to use chrono functions
#include <chrono>
#include <queue> // Image queue
#include <semaphore.h>
#include <mutex>
#include <thread>
#include <numeric>

/*
Naming Conventions
variables are named using snake_case
global variables are ALL_CAPS
methods use camelCase

TO USE PERIOD OPTIMIZATION, MAX FREQ AND MIN FREQ MUST BE WHOLE NUMBERS

CHANGE MAX_HILBERT SIZE IF ORDER SIZE > 5
*/

using ValueType = std::variant<int, double, std::string, bool>;
using ChronoType = std::chrono::time_point<std::chrono::high_resolution_clock>;

// -------------------------------- CHANGE THIS IF USING ORDER OF SIZE > 5
constexpr size_t MAX_HILBERT_SIZE = 1024;

// Setting variables
int ORDER = 3; // order of hilbert curve, dimensions are determined from this
double MIN_FREQ = 100; // lowest frequency
double MAX_FREQ = 500; // highest frequency
double DURATION = 5; // duration in seconds
int SAMPLE_RATE = 22000; // number of samples per second
std::string IMAGE_URL = "white.png"; // url to test image
std::string WAV_FILENAME = "sound.wav";
double VOLUME = 1; // 0 - 1
bool PLAY_AUDIO = 1;
bool CAP_FROM_CAMERA = 1;
bool SAVE_IMG = 0;
int CAM_1_INDEX = 0;
int CAM_2_INDEX = 1;
int N_RUNS = 1;
bool PERIOD_OPTIMIZATION = false;
double FADE_PERCENT = 0.01;
bool DEBUG = false;
double minDisparity = 0;
int numDisparity = 16;

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
  int fade_samples = (int) ((double) sample_count * FADE_PERCENT);
  std::cout << "Fade in first " << fade_samples << "samples" << std::endl;

  for (int i = 0; i < sample_count; i++) {
    float sample = 0;
    for (int n = 0; n < n_pixel; n++) {
        sample += std::sin(2.0f * M_PI * freqs[n] * static_cast<float>(i) / sample_rate) * volumes[n];
    } 
    double fade_in = ((double) std::min(i, fade_samples)) / fade_samples;
    double fade_out = ((double) std::min(sample_count - i, fade_samples)) / fade_samples;
    samples[i] = static_cast<short>((sample / n_pixel) * 32767 * std::pow(fade_in, 2) * std::pow(fade_out, 2));
    // std::cout << samples[i] << std::endl;
  } 
}

void generateHilbert(struct Pair hilbert[], int n_pixels) {
  for (int i = 0; i < n_pixels; i++) {
    hilbert[i] = getCoord(i);
  }
}

/** Return: A double representing the period of the sum of sines using these frequencies
 */
double generateFrequencies(float freqs[], double min_freq, double max_freq, int n_pixels) {
  int lcm_num = n_pixels;
  int gcd_den = min_freq * n_pixels;
  for (int i = 0; i < n_pixels; i++) {
    int numerator = i * (max_freq - min_freq) + min_freq * n_pixels;
    int denominator = n_pixels;
    freqs[i] = numerator / denominator; // I know its the wrong way around, its fine.
    
    // Period is 1 / f, so flip numerator and denominator
    // T = lcm(d1, d2, ... dn) / gcd(n1, n2, ..., nn) | fi = ni / di
    gcd_den = std::gcd(gcd_den, numerator);
  }

  return (double) lcm_num / gcd_den;
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


// -----------------------------------Thread Functions---------------------------------------

void imageGen(cv::VideoCapture& cap, int image_width, std::queue<cv::Mat>& img_queue, sem_t& img_sem, std::mutex& img_mutex) {
  const int MAX_IMAGES = 2;

  cv::Mat image;

  std::cout << "image gen thread start" << std::endl;

  for (int i = 0; i < N_RUNS; i++) {
    // Capture image
    captureImage(image, cap, image_width);

    // Push image pointer to queue
    while (img_queue.size() >= MAX_IMAGES) {}
    
    // When room in queue
    {
      std::lock_guard<std::mutex> lock(img_mutex);
      img_queue.push(image);
      // std::cout << i << ": Pushed to image queue" << std::endl;
    }
    
    // Post semaphore 
    sem_post(&img_sem);
  }

  std::cout << "image gen thread end" << std::endl;
}

template <size_t N>
void audioGen(std::queue<cv::Mat>& img_queue, sem_t& img_sem, std::mutex& img_mutex, int n_pixels, Pair (&hilbert)[N], float (&freqs)[N], ALuint& source, std::mutex& audio_mutex, sem_t& audio_sem) {

  const int MAX_BUFFERS = 2;

  cv::Mat image;
  short* samples = nullptr;
  int sample_count = genSampleArray(samples, SAMPLE_RATE, DURATION);
  
  std::cout << "audio gen thread start" << std::endl;

  for (int i = 0; i < N_RUNS; i++) {
    // Wait until an image is available in the queue
    sem_wait(&img_sem);
    {
      std::lock_guard<std::mutex> lock(img_mutex);
      image = img_queue.front();
      img_queue.pop();
      // std::cout << i << ": Removed image from queue" << std::endl;
    }

    // Check if failed to get image

    // Get volumes
    float volumes[n_pixels];
    generateVolumes(volumes, image, hilbert, n_pixels);

    // Generate sines
    generateSines(samples, sample_count, n_pixels, SAMPLE_RATE, volumes, freqs);

    // Create buffer
    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, samples, sample_count * sizeof(short), SAMPLE_RATE);

    // Push buffer onto queue
    ALint buffers_queued;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &buffers_queued);
    while (buffers_queued >= MAX_BUFFERS) {
      alGetSourcei(source, AL_BUFFERS_QUEUED, &buffers_queued);  
    }

    {
      std::lock_guard<std::mutex> lock_guard(audio_mutex);
      alSourceQueueBuffers(source, 1, &buffer);
      // std::cout << i << ": Pushed buffer onto queue" << std::endl;
    }
    sem_post(&audio_sem);
  }

  delete[] samples;

  std::cout << "audio gen thread end" << std::endl;

}

void audioPlay(sem_t& audio_sem, ALuint& source) {

  std::cout << "audio play thread start" << std::endl;

  for (int i = 0; i < N_RUNS; i++) {
    sem_wait(&audio_sem);
      
    ALint source_state;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    alSourcei(source, AL_LOOPING, ((PERIOD_OPTIMIZATION) ? AL_TRUE : AL_FALSE));
    if (source_state != AL_PLAYING) {
      // If the source isn't playing, start playback (or restart if needed)
      alSourcePlay(source);
    }

    ChronoType start_time = std::chrono::high_resolution_clock::now();
    
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    while (source_state == AL_PLAYING) {
      if (PERIOD_OPTIMIZATION) {
        ChronoType current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current_time - start_time;
        if (elapsed.count() >= DURATION){
          alSourcei(source, AL_LOOPING, AL_FALSE);
          break;
        }
      }
      alGetSourcei(source, AL_SOURCE_STATE, &source_state);
      // std::cout << "PLAYING" << std::endl;
    }

    std::cout << "Audio ended" << std::endl;

    // Pop processed audio from queue
    ALint processed;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    
    while(processed > 0) {
      ALuint processed_buffer;
      alSourceUnqueueBuffers(source, 1, &processed_buffer);
      alDeleteBuffers(1, &processed_buffer);
      // std::cout << i << ": Popped buffer from queue" << std::endl;
      --processed;
    }

  }

  std::cout << "audio play thread end" << std::endl;

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
      getValue("WAV_FILENAME", WAV_FILENAME);
      getValue("VOLUME", VOLUME);
      getValue("PLAY_AUDIO", PLAY_AUDIO);
      getValue("CAP_FROM_CAMERA", CAP_FROM_CAMERA);
      getValue("SAVE_IMG", SAVE_IMG);
      getValue("CAM_1_INDEX", CAM_1_INDEX);
      getValue("CAM_2_INDEX", CAM_2_INDEX);
      getValue("N_RUNS", N_RUNS);
      getValue("PERIOD_OPTIMIZATION", PERIOD_OPTIMIZATION);
      getValue("FADE_PERCENT", FADE_PERCENT);
      getValue("minDisparity", minDisparity);
      getValue("numDisparity", numDisparity);

    } else {
      std::cout << "Settings file failed to open\n Default settings applied" << std::endl;
      // leave default settings
    }
    settings.close();
    
  // ------------------------------------------------Derive variables, Open camera-------------make
  // --   
    int image_width = std::pow(2, ORDER);
    int n_pixels = image_width * image_width;
    cv::VideoCapture cap(CAM_1_INDEX);

    if (!cap.isOpened()) {
      std::cerr << "Error: Could not open webcam" << std::endl;
      return -1;
    }
  // ------------------------------------------------Generate hilbert pair array------------------------
    struct Pair hilbert[MAX_HILBERT_SIZE];
    generateHilbert(hilbert, n_pixels);
    std::cout << "hilbert done" << std::endl;
  // ------------------------------------------------Generate frequency array------------------------------------------------
    float freqs[MAX_HILBERT_SIZE];
    double period = generateFrequencies(freqs, MIN_FREQ, MAX_FREQ, n_pixels);
    if (PERIOD_OPTIMIZATION && period < DURATION) {
      DURATION = period;
      std::cout << "OPTIMIZATION: Using period optimization" << std::endl;
    }
    std::cout << "Period: " << period << std::endl;

    std::cout << "freq done" << std::endl;

  // ------------------------------------------------Sine waves------------------------------------------------
    float sine[1]; // needs sample count

    // GOAL: generate an array of samples

    std::cout << "sine waves done" << std::endl;

  
  cv::Mat image;
  
  ALCdevice* device = nullptr;
  ALCcontext *context = nullptr;
  // Open device
  device = alcOpenDevice(nullptr); // open default device
  if (!device) {
      std::cerr << "Error: Could not open sound device." << std::endl;
      return -1;
  }

  std::cout << "open al device done" << std::endl;

  // Create context
  context = alcCreateContext(device, nullptr);
    if (!context || !alcMakeContextCurrent(context)) {
        std::cerr << "Error: Could not create or set context." << std::endl;
        if (context) alcDestroyContext(context);
        alcCloseDevice(device);
        return -1;
    }
  std::cout << "create al context done" << std::endl;

  // Set source  
  ALuint source;
  alGenSources(1, &source);

  std::mutex img_mutex;
  std::mutex audio_mutex;

  sem_t img_sem;
  sem_t aud_sem;
  sem_init(&img_sem, 0, 0);
  sem_init(&aud_sem, 0, 0);
  
  std::queue<cv::Mat> img_queue;

  // Create camera capture thread
  std::thread img_gen([&]() {
    imageGen(cap, image_width, img_queue, img_sem, img_mutex);
  });

  // Create audio generator thread
  std::thread aud_gen([&]() {
    audioGen<MAX_HILBERT_SIZE>(img_queue, img_sem, img_mutex, n_pixels, hilbert, freqs, source, audio_mutex, aud_sem);
  });
    
  // Create audio player thread
  std::thread aud_ply([&]() {
    audioPlay(aud_sem, source);
  });
  
  img_gen.join();
  aud_gen.join();
  aud_ply.join();

  // Delete the generated sine wave data
  sem_destroy(&img_sem);
  sem_destroy(&aud_sem);
  
  alDeleteSources(1, &source);// Close OpenAL context and device
  alcMakeContextCurrent(nullptr);
  alcDestroyContext(context);
  alcCloseDevice(device);
  std::cout << "al context closed" << std::endl;
  
  return 0;
}