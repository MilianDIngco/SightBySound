#include "audiomanager.hpp"
#include "settings.hpp"
#include <cmath>
#include <numeric>
#include <iostream>

void AudioManager::generateSampleArray(std::vector<short> &samples, int sample_rate, float duration) {
  if (sample_rate == -1) {
    sample_rate = this->sample_rate;
  }
  if (duration == -1) {
    duration = this->duration;
  }
  int sample_count = static_cast<int>(sample_rate * duration);

  samples.resize(sample_count);
}

double AudioManager::generateFrequencies(std::vector<float> &frequencies, double min_freq, double max_freq, int n_points) {
  int lcm_num = n_points;
  int gcd_den = min_freq * n_points;
  frequencies.resize(n_points);
  for (int i = 0; i < n_points; i++) {
    float numerator = i * (max_freq - min_freq) + min_freq * n_points;
    float denominator = n_points;
    frequencies.at(i) = numerator / denominator; 

    // Period is 1 / f, so flip numerator and denominator
    // T = lcm(d1, d2, ... dn) / gcd(n1, n2, ..., nn) | fi = ni / di
    gcd_den = std::gcd(gcd_den, (int) numerator);
  }

  return (double)lcm_num / gcd_den;
}

double AudioManager::generateSines(std::vector<short> &samples, std::vector<float> &volumes, double phase) {
  int sample_count = samples.size();

  int fade_samples = (int)((double)sample_count * this->fade_percent);

  if (phase > this->period) {
    phase = 0;
  }
  int start_sample = phase * this->sample_rate;

  for (int i = start_sample; i < start_sample + sample_count; i++) {
    double sample = 0;
    for (int n = 0; n < this->frequencies.size(); n++) {
      sample += std::sin(2.0f * M_PI * this->frequencies[n] * static_cast<double>(i) / sample_rate) * volumes[n];
    }

    // Normalize then softmax the sample
    sample /= this->frequencies.size();
    sample = this->soft_max(sample, this->audio_soften, this->audio_threshold);
    // i that indexes into samples array
    int sample_i = i - start_sample;

    double fade_in = ((double)std::min(sample_i, fade_samples)) / fade_samples;
    double fade_out = ((double)std::min(sample_count - sample_i, fade_samples)) / fade_samples;
    samples[sample_i] = static_cast<short>(sample * 32767 * std::pow(fade_in, 2) * std::pow(fade_out, 2) * this->volume);
  }

  return phase + static_cast<double>(samples.size()) / this->sample_rate;
}

double AudioManager::soft_max(double sample, double soften, double threshold) {
  double abs_sample = std::abs(sample);
  if (abs_sample < threshold) { return sample * this->audio_max; }

  double excess = abs_sample - threshold;
  double soft = this->audio_max * (std::pow(abs_sample, soften) * excess + threshold);
  return (sample < 0) ? -soft : soft;
}

AudioManager::AudioManager(Settings settings) {
  this->sample_rate = settings.sample_rate;
  this->duration = settings.duration;
  this->min_freq = settings.min_freq;
  this->max_freq = settings.max_freq;
  this->fade_percent = settings.fade_percent;
  this->volume = settings.volume;
  this->audio_threshold = settings.audio_threshold;
  this->audio_soften = settings.audio_soften;
  this->audio_max = settings.audio_max;

  int n_points = std::pow(2, 2 * settings.order);
  this->period = this->generateFrequencies(this->frequencies, this->min_freq, this->max_freq, n_points);
}
