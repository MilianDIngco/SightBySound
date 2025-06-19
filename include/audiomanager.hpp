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

  public:
    void generateSampleArray(std::vector<short> samples, int sample_rate = -1, float duration = -1);

    double generateFrequencies(std::vector<float> frequencies, double min_freq, double max_freq, int n_points);

    void generateSines(std::vector<short> samples, std::vector<float> volumes, int sample_rate = -1, int phase = 0);

    AudioManager(int sample_rate, float duration, double min_freq, double max_freq, double fade_percent, double volume, int order);
    AudioManager(Settings settings) : AudioManager(settings.sample_rate, settings.duration, settings.min_freq, settings.max_freq, settings.fade_percent, settings.volume, settings.order) {};
    AudioManager() : AudioManager(22000, 0.1, 30, 200, 0.1, 1, 3) {};

    // add play audio stuff
    

};

#endif // !AUDIOGENERATOR_H
