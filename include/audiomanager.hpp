#ifndef AUDIOGENERATOR_H
#define AUDIOGENERATOR_H

#include "settings.hpp"
#include <vector> 

class AudioManager {
  private:
    std::vector<float> frequencies;
    int sample_rate;
    float duration;
    double period;
    double min_freq;
    double max_freq;
    double fade_percent;
    double volume;
    double audio_threshold;
    double audio_soften;
    double audio_max;

  public:
    void generateSampleArray(std::vector<short> &samples, int sample_rate = -1, float duration = -1);

    double generateFrequencies(std::vector<float> &frequencies, double min_freq, double max_freq, int n_points);

    double generateSines(std::vector<short> &samples, std::vector<float> volumes, double phase);

    double soft_max(double sample, double soften, double threshold);

    AudioManager(Settings settings);

    int get_sample_rate() { return this->sample_rate; };
    float get_duration() { return this->duration; };
    double get_period() { return this->period; };
    double get_min_freq() { return this->min_freq; };
    double get_max_freq() { return this->max_freq; };
    double get_fade_percent() { return this->fade_percent; };
    double get_volume() { return this->volume; };
    std::vector<float> get_frequencies() { return this->frequencies; };   
    double get_audio_threshold() { return this->audio_threshold; };
    double get_audio_soften() { return this->audio_soften; };
    double get_audio_max() { return this->audio_max; };

};

#endif // !AUDIOGENERATOR_H
