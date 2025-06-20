#ifndef SIGHTBYSOUND_H
#define SIGHTBYSOUND_H

#include "audiomanager.hpp"
#include "cameradepth.hpp"
#include "hilbert.hpp"
#include "opencv2/core/mat.hpp"
#include "settings.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <memory>
#include <semaphore.h>
#include <queue>
#include <string>
#include <vector>
class SightBySound {
  private:
    std::string settings_path;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<AudioManager> audio_manager;
    std::unique_ptr<CameraDepth> camera_depth;
    std::unique_ptr<Hilbert> hilbert;
    std::queue<cv::Mat> lr_img_queue;
    std::queue<cv::Mat> img_queue;
    sem_t lr_img_sem;
    sem_t img_sem;
    sem_t audio_sem;
    std::mutex lr_img_mutex;
    std::mutex img_mutex;
    std::mutex audio_mutex;
    std::mutex print_mutex;
    std::vector<ALuint> free_buffers;
    ALuint source;
    ALCdevice *device;
    ALCcontext *context;
    int n_runs;
    int max_buffer;
    bool debug_print;

    void imageGen(CameraDepth camera_depth, std::queue<cv::Mat> &lr_img_queue, sem_t &lr_img_sem, std::mutex &lr_img_mutex);
    void depthGen(CameraDepth camera_depth, std::queue<cv::Mat> &lr_img_queue, sem_t &lr_img_sem, std::mutex &lr_img_mutex,
                  std::queue<cv::Mat> &img_queue, sem_t &img_sem, std::mutex &img_mutex);
    void audioGen(Hilbert hilbert, AudioManager audio_manager, std::queue<cv::Mat> &img_queue, sem_t &img_sem, std::mutex &img_mutex,
                  std::vector<ALuint>&free_buffers, ALuint &source, std::mutex &audio_mutex, sem_t &audio_sem);
    void audioPlay(AudioManager audio_manager, std::vector<ALuint> &free_buffers, ALuint &source, std::mutex &audio_mutex, sem_t &audio_sem);
    void debugPrint(const std::string str);

  public: 
    SightBySound(const std::string &settings_path) {
      this->settings_path = settings_path;

      // Set up classes
      this->settings = std::make_unique<Settings>(settings_path);
      this->debugPrint("Settings finished");
      this->audio_manager = std::make_unique<AudioManager>(*this->settings);
      this->camera_depth = std::make_unique<CameraDepth>(*this->settings);
      this->hilbert = std::make_unique<Hilbert>(*this->settings);

      // Set up instance variables
      this->debug_print = settings->debug_print;

      // Set up openAL
      ALCdevice *device = nullptr;
      ALCcontext *context = nullptr;

      device = alcOpenDevice(nullptr);
      if (!device) {
        std::cerr << "ERROR: Could not open sound device." << std::endl;
        return;
      }

      context = alcCreateContext(device, nullptr);
      if (!context || !alcMakeContextCurrent(context)) {
        std::cerr << "ERROR: Could not create or set context" << std::endl;
        if (context) 
          alcDestroyContext(context);
        alcCloseDevice(device);
        return;
      }

      this->device = device;
      this->context = context;

      alGenSources(1, &this->source);
      this->debugPrint("Finished OpenAL setup");
      
      sem_init(&this->lr_img_sem, 0, 0);
      sem_init(&this->img_sem, 0, 0);
      sem_init(&this->audio_sem, 0, 0);

      this->free_buffers.resize(this->settings->max_buffer);

      this->n_runs = settings->n_runs;
    };

    ~SightBySound();
    void run();
};

#endif // !SIGHTBYSOUND_H
