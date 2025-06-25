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
    bool save_img;
    std::string save_img_path;

    void imageGen(CameraDepth camera_depth, std::queue<cv::Mat> &lr_img_queue, sem_t &lr_img_sem, std::mutex &lr_img_mutex);
    void depthGen(CameraDepth camera_depth, std::queue<cv::Mat> &lr_img_queue, sem_t &lr_img_sem, std::mutex &lr_img_mutex,
                  std::queue<cv::Mat> &img_queue, sem_t &img_sem, std::mutex &img_mutex);
    void audioGen(Hilbert hilbert, AudioManager audio_manager, std::queue<cv::Mat> &img_queue, sem_t &img_sem, std::mutex &img_mutex,
                  std::vector<ALuint>&free_buffers, ALuint &source, std::mutex &audio_mutex, sem_t &audio_sem);
    void audioPlay(AudioManager audio_manager, std::vector<ALuint> &free_buffers, ALuint &source, std::mutex &audio_mutex, sem_t &audio_sem);
    void debugPrint(const std::string str);

  public: 

    std::unique_ptr<AudioManager> audio_manager;
    std::unique_ptr<CameraDepth> camera_depth;
    std::unique_ptr<Hilbert> hilbert;

    SightBySound(const std::string &settings_path);
    ~SightBySound();
    void run();
};

#endif // !SIGHTBYSOUND_H
