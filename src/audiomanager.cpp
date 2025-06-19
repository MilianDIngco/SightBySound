#include "../include/audiomanager.hpp"
#include <cmath>
#include <numeric>

void AudioManager::generateSampleArray(std::vector<short> samples, int sample_rate, float duration) {
  if (sample_rate == -1) {
    sample_rate = this->sample_rate;
  }
  if (duration == -1) {

  }
  int sample_count = static_cast<int>(sample_rate * duration);

  samples.resize(sample_count);
}

double AudioManager::generateFrequencies(std::vector<float> frequencies, double min_freq, double max_freq, int n_points) {
  int lcm_num = n_points;
  int gcd_den = min_freq * n_points;
  for (int i = 0; i < n_points; i++) {
    float numerator = i * (max_freq - min_freq) + min_freq * n_points;
    float denominator = n_points;
    frequencies[i] = numerator / denominator; 

    // Period is 1 / f, so flip numerator and denominator
    // T = lcm(d1, d2, ... dn) / gcd(n1, n2, ..., nn) | fi = ni / di
    gcd_den = std::gcd(gcd_den, (int) numerator);
  }

  return (double)lcm_num / gcd_den;
}

void AudioManager::generateSines(std::vector<short> samples, std::vector<float> volumes, int sample_rate, int phase) {
  if (sample_rate == -1) {
    sample_rate = this->sample_rate;
  }
  int sample_count = samples.size();

  int fade_samples = (int)((double)sample_count * this->fade_percent);

  for (int i = 0; i < sample_count; i++) {
    float sample = 0;
    for (int n = 0; n < this->frequencies.size(); n++) {
      sample += std::sin(2.0f * M_PI * this->frequencies[n] * static_cast<float>(i) / sample_rate) * volumes[n];
    }

    // i that indexes into samples array
    int sample_i = i;

    double fade_in = ((double)std::min(sample_i, fade_samples)) / fade_samples;
    double fade_out = ((double)std::min(sample_count - sample_i, fade_samples)) / fade_samples;
    samples[sample_i] = static_cast<short>((sample / this->frequencies.size()) * 32767 * std::pow(fade_in, 2) * std::pow(fade_out, 2) * this->volume);
  }
}

AudioManager::AudioManager(int sample_rate, float duration, double min_freq, double max_freq, double fade_percent, double volume, int order) {
  this->sample_rate = sample_rate;
  this->duration = duration;
  this->min_freq = min_freq;
  this->max_freq = max_freq;
  this->fade_percent = fade_percent;
  this->volume = volume;

  int n_points = std::pow(2, 2 * order);
  this->generateFrequencies(this->frequencies, this->min_freq, this->max_freq, n_points);
}
