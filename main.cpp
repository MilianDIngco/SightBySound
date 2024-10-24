#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>  
#include <AL/al.h>
#include <AL/alc.h>

const int order = 2;
const int square_size = 4; 

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

Pair getCoord(int index) {
    // bounds checking; Number of points is ( 2 ^ order ) ^ 2 since it's a square
    // array for now
    if (index >= std::pow(4, order))
      return Pair(-1, -1);

    Pair order1[] = {
      Pair(0, 0), Pair(0, 1),
      Pair(1, 1), Pair(1, 0)
    };

    // Depth of recursion is mapped by (log base 4) + 1
    // Index < 4 : order 1
    // Index < 16 : order 2 ...
    int depth = order;

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

void genSines(short* samples, int sample_count, int n_pixel, int sample_rate, float* volumes, float* freqs) {
  std::cout << sample_count << " " << n_pixel << " " << sample_count * n_pixel << std::endl;
    for (int i = 0; i < sample_count; i++) {
        float sample = 0;
        for (int n = 0; n < n_pixel; n++) {
            sample += std::sin(2.0f * M_PI * freqs[n] * static_cast<float>(i) / sample_rate) * volumes[n];
        }
        
        // if (i % 100 == 0) {
        //   std::cout << i << std::endl;
        // }  
        samples[i] = static_cast<short>(sample * 32767 / n_pixel);
    
        std::cout << samples[i] << std::endl;
    }
} 

int main(int argc, char** argv) {

  // Generate hilbert pair array
  struct Pair hilbert[square_size];
  for (int i = 0; i < square_size; i++) {
    hilbert[i] = getCoord(i);
    // std::cout << hilbert[i].toString() << std::endl;
  }
  std::cout << "hilbert done" << std::endl;
  // Generate frequency array
  float freqs[square_size];
  float min_freq = 100;
  float max_freq = 400;
  float freq_step = (max_freq - min_freq) / square_size;
  for (int i = 0; i < square_size; i++) {
    freqs[i] = min_freq + (i * freq_step);
    std::cout << freqs[i] << std::endl;
  }
  std::cout << "freq done" << std::endl;
  // Capture image
  // Open jpg file
  cv::Mat image = cv::imread("apple.png");

  // TO DO: IMAGE PROCESSING

  // First, try just using a gray scale image
  cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
  std::cout << "image done" << std::endl;
  // Then, if unclear try the depth images

  // Create volume array from image
  float volumes[square_size];
  float max_volume = 1.0f; // ranges from 0 to 1
  for (int i = 0; i < square_size; i++) {
    float pixel_value = static_cast<float>(image.at<uchar>(hilbert[i].x, hilbert[i].y));
    volumes[i] = (pixel_value / 255.0f) * max_volume;
    // std::cout << volumes[i] << std::endl;
  }
  std::cout << "volume done" << std::endl;


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

  // Generate Sine Wave and load into buffer
  float duration = 1.0f;
  short* samples = nullptr;
  int sample_rate = 1000;
  int sample_count = genSampleArray(samples, sample_rate, duration);
  genSines(samples, sample_count, square_size, sample_rate, volumes, freqs);
  std::cout << "sine waves done" << std::endl;
  // Play sound

  // Create a buffer and fill it with the generated sine wave data
  ALuint buffer;
  alGenBuffers(1, &buffer);
  alBufferData(buffer, AL_FORMAT_MONO16, samples, sample_count * sizeof(short), sample_rate);
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

  // Delete the generated sine wave data
  delete[] samples;
  std::cout << "samples deleted done" << std::endl;

  return 0;
}