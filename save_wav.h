#ifndef SAVE_WAV_H
#define SAVE_WAV_H

#include <string>
#include <vector>

void saveWav(const std::string &filename, const std::vector<short> &samples, int sampleRate, int numChannels);

void instantiateWav(const std::string &filename, int sample_rate, int num_channels);

void appendWav(const std::string &filename, const std::vector<short> &samples);

#endif // SAVE_WAVE_H