#ifndef SAVE_WAV_H
#define SAVE_WAV_H

#include <string>
#include <vector>

void saveWav(const std::string &filename, const std::vector<short> &samples, int sampleRate, int numChannels);

#endif // SAVE_WAVE_H