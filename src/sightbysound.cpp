#include "sightbysound.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <opencv2/core/hal/interface.h>
#include <semaphore.h>
#include <thread>

void SightBySound::debugPrint(const std::string str) {
  if (this->debug_print) {
    std::lock_guard<std::mutex> lock(this->print_mutex);
    std::cout << str << std::endl;
  }
}

SightBySound::~SightBySound() {
  sem_destroy(&this->lr_img_sem);
  sem_destroy(&this->img_sem);
  sem_destroy(&this->audio_sem);

  alDeleteSources(1, &this->source);
  alcMakeContextCurrent(nullptr);
  alcDestroyContext(this->context);
  alcCloseDevice(this->device);
}

void SightBySound::run() {

  this->debugPrint("Starting threads");

  // Initialize threads
  std::thread img_gen([&]() {
    this->imageGen(*this->camera_depth, this->lr_img_queue, this->lr_img_sem, this->lr_img_mutex);
  });

  std::thread dep_gen([&]() {
    this->depthGen(*this->camera_depth, this->lr_img_queue, this->lr_img_sem, this->lr_img_mutex,
             this->img_queue, this->img_sem, this->img_mutex);
  });

  std::thread aud_gen([&]() {
    this->audioGen(*this->hilbert, *this->audio_manager, this->img_queue, this->img_sem, this->img_mutex, 
             this->free_buffers, this->source, this->audio_mutex, this->audio_sem);
  });

  std::thread aud_ply([&]() {
    this->audioPlay(*this->audio_manager, this->free_buffers, this->source, this->audio_mutex, this->audio_sem);
  });

  img_gen.join();
  this->debugPrint("Image gen finished");
  dep_gen.join();
  this->debugPrint("Depth gen finished");
  aud_gen.join();
  this->debugPrint("Audio gen finished");
  aud_ply.join();
  this->debugPrint("Audio play finished");
}

void SightBySound::imageGen(CameraDepth camera_depth, std::queue<cv::Mat> &lr_img_queue, sem_t &lr_img_sem, std::mutex &lr_img_mutex) 
{
  int max_buffer = this->max_buffer;

  for (int i = 0; i < this->n_runs; i++) {
    cv::Mat left_frame, right_frame;

    camera_depth.captureImages(left_frame, right_frame);

    camera_depth.rectifyImages(left_frame, right_frame);

    camera_depth.grayImages(left_frame, right_frame);

    camera_depth.scaleImages(left_frame, camera_depth.prestereo_scale);
    camera_depth.scaleImages(right_frame, camera_depth.prestereo_scale);

    while (lr_img_queue.size() >= max_buffer * 2) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    {
      std::lock_guard<std::mutex> lock(lr_img_mutex);
      lr_img_queue.push(right_frame);
      lr_img_queue.push(left_frame);
    }

    sem_post(&lr_img_sem);
  }
}

void SightBySound::depthGen(CameraDepth camera_depth, std::queue<cv::Mat> &lr_img_queue, sem_t &lr_img_sem, std::mutex &lr_img_mutex, 
                            std::queue<cv::Mat> &img_queue, sem_t &img_sem, std::mutex &img_mutex) 
{
  for (int i = 0; i < this->n_runs; i++) {
    cv::Mat left_frame, right_frame, depth;

    sem_wait(&lr_img_sem);
    {
      std::lock_guard<std::mutex> lock(lr_img_mutex);

      right_frame = lr_img_queue.front();
      lr_img_queue.pop();

      left_frame = lr_img_queue.front();
      lr_img_queue.pop();
    }

    depth = camera_depth.getDepthImage(left_frame, right_frame);

    camera_depth.scaleImages(depth, camera_depth.hilbert_scale);

    while (img_queue.size() >= this->max_buffer) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    {
      std::lock_guard<std::mutex> lock(img_mutex);
      img_queue.push(depth);
    }

    sem_post(&img_sem);
  }
}

void SightBySound::audioGen(Hilbert hilbert, AudioManager audio_manager, std::queue<cv::Mat> &img_queue, sem_t &img_sem, std::mutex &img_mutex, 
                            std::vector<ALuint>&free_buffers, ALuint &source, std::mutex &audio_mutex, sem_t &audio_sem) 
{
  constexpr ALuint empty_buffer = 0;

  for (int i = 0; i < this->n_runs; i++) {
    cv::Mat image;
    std::vector<uchar> image_vector;
    std::vector<uchar> pixel_values;

    sem_wait(&img_sem);
    {
      std::lock_guard<std::mutex> lock(img_mutex);
      image = img_queue.front();
      img_queue.pop();
    }

    image_vector.assign(image.begin<uchar>(), image.end<uchar>());
    pixel_values = hilbert.toHilbert(image_vector);

    std::vector<float> volumes(pixel_values.size());
    for (int i = 0; i < volumes.size(); i++) {
      volumes[i] = pixel_values[i] / 255.0f;
    }

    std::vector<short> samples;
    audio_manager.generateSampleArray(samples);
    audio_manager.generateSines(samples, volumes);

    ALint buffers_queued;
    alGetSourcei(source, AL_BUFFERS_QUEUED, &buffers_queued);
    while (buffers_queued >= this->max_buffer) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      alGetSourcei(source, AL_BUFFERS_QUEUED, &buffers_queued);
    }

    {
      std::lock_guard<std::mutex> lock(audio_mutex);

      ALuint buffer;
      for (int i = 0; i < this->max_buffer; i++) {
        if (free_buffers[i] != empty_buffer) {
          buffer = free_buffers[i];
          free_buffers[i] = empty_buffer;
          break;
        }
      }

      alBufferData(buffer, AL_FORMAT_MONO16, samples.data(), samples.size() * sizeof(short), audio_manager.get_sample_rate());
      alSourceQueueBuffers(source, 1, &buffer);
    }
    sem_post(&audio_sem);
  }
}

void SightBySound::audioPlay(AudioManager audio_manager, std::vector<ALuint> &free_buffers, ALuint &source, 
                             std::mutex &audio_mutex, sem_t &audio_sem) 
{
  constexpr ALuint empty_buffer = 0;

  for (int i = 0; i < this->n_runs; i++) {
    sem_wait(&audio_sem);

    ALint source_state;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    if (source_state != AL_PLAYING) {
      alSourcePlay(source);
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    while (source_state == AL_PLAYING) {
      alGetSourcei(source, AL_SOURCE_STATE, &source_state);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ALint processed;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    {
      std::lock_guard<std::mutex> lock(audio_mutex);

      while (processed > 0) {
        ALuint processed_buffer;
        alSourceUnqueueBuffers(source, 1, &processed_buffer);

        for (int i = 0; i < this->max_buffer; i++) {
          if (free_buffers[i] == empty_buffer) {
            free_buffers[i] = processed_buffer;
            break;
          }
        }

        --processed;
      }
    }

  }

}
