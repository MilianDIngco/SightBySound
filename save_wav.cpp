#include "save_wav.h"
#include <fstream>
#include <iostream>
#include <climits>

/*
WAV file header wikipedia
https://en.wikipedia.org/wiki/WAV

*/


void saveWav(const std::string &filename, const std::vector<short> &samples, int sample_rate, int num_channels) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "ERROR: Failed to open " << filename << " for writing" << std::endl;
        return;
    }

    int bits_per_sample = sizeof(short) * CHAR_BIT; // Size of returns 2 bytes & multiplies by 8 (bits in a byte) to get 16 bits per sample
    int data_size = samples.size() * sizeof(short); // Gets size of samples in bytes
    int chunk_size = 36 + data_size; // 36 bc. FileSize takes the overall file size minus 8 bytes and the header size is 44 bytes tf 44-8=36

    // Master RIFF chunk
    file.write("RIFF", 4);                                          // FileTypeBlocID   (4 bytes) : Identifier "RIFF"
    file.write(reinterpret_cast<const char *>(&chunk_size), 4);     // FileSize         (4 bytes) : Overall file size minus 8 bytes
        // ^ uses reinterpret_cast to access the direct memory and treat the int pointer to chunk_size as a char pointer since thats what write() wants
    file.write("WAVE", 4);                                          // FileFormatID     (4 bytes) : Format = "WAVE"

    // Chunk that describes the data format
    file.write("fmt ", 4);                                          // FormatBlocID     (4 bytes) : Identifier "fmt "
    
    int fmt_chunk_size = 16; // 24 bytes - 8 bytes
    file.write(reinterpret_cast<const char *>(&fmt_chunk_size), 4); // BlocSize         (4 bytes) : Chunk size minus 8 bytes

    int audio_format = 1; // Using PCM integer format
    file.write(reinterpret_cast<const char *>(&audio_format), 2);   // Audio_format     (2 bytes) : 1: PCM integer, 3: IEEE 754 float
    file.write(reinterpret_cast<const char *>(&num_channels), 2);   // NbrChannels      (2 bytes) : # of channels
    file.write(reinterpret_cast<const char *>(&sample_rate), 4);    // Frequency        (4 bytes) : Sample rate in hertz

    int byte_per_bloc = num_channels * (bits_per_sample / CHAR_BIT); // number of bytes in a single complete audio frame
    int byte_per_sec = sample_rate * byte_per_bloc; // # of bytes / second to read
    file.write(reinterpret_cast<const char *>(&byte_per_sec), 4);   // BytePerSec       (4 bytes) : # of bytes to read per second
    file.write(reinterpret_cast<const char *>(&byte_per_bloc), 2);  // BytePerBloc      (2 bytes) : # of bytes per bloc (a single complete audio frame (a single frame from all channels) )
    file.write(reinterpret_cast<const char *>(&bits_per_sample), 2);// BitsPerSample    (2 bytes) : # of bits per sample

    // Chunk containing the sampled data
    file.write("data", 4);                                          // DataBlocID       (4 bytes) : Identifier "data"
    file.write(reinterpret_cast<const char *>(&data_size), 4);      // DataSize         (4 bytes) : SampledData size

    // writing all of the audio samples
    file.write(reinterpret_cast<const char *>(samples.data()), data_size);

    file.close();
    std::cout << "WAV file saved: " << filename << std::endl;

} 

void instantiateWav(const std::string &filename, int sample_rate, int num_channels) {
    int size = 0;

    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "ERROR: Failed to open " << filename << " for writing" << std::endl;
        return;
    }

    int bits_per_sample = sizeof(short) * CHAR_BIT; // Size of returns 2 bytes & multiplies by 8 (bits in a byte) to get 16 bits per sample
    int data_size = size * sizeof(short); // Gets size of samples in bytes
    int chunk_size = 36 + data_size; // 36 bc. FileSize takes the overall file size minus 8 bytes and the header size is 44 bytes tf 44-8=36

    // Master RIFF chunk
    file.write("RIFF", 4);                                          // FileTypeBlocID   (4 bytes) : Identifier "RIFF"
    file.write(reinterpret_cast<const char *>(&chunk_size), 4);     // FileSize         (4 bytes) : Overall file size minus 8 bytes
        // ^ uses reinterpret_cast to access the direct memory and treat the int pointer to chunk_size as a char pointer since thats what write() wants
    file.write("WAVE", 4);                                          // FileFormatID     (4 bytes) : Format = "WAVE"

    // Chunk that describes the data format
    file.write("fmt ", 4);                                          // FormatBlocID     (4 bytes) : Identifier "fmt "
    
    int fmt_chunk_size = 16; // 24 bytes - 8 bytes
    file.write(reinterpret_cast<const char *>(&fmt_chunk_size), 4); // BlocSize         (4 bytes) : Chunk size minus 8 bytes

    int audio_format = 1; // Using PCM integer format
    file.write(reinterpret_cast<const char *>(&audio_format), 2);   // Audio_format     (2 bytes) : 1: PCM integer, 3: IEEE 754 float
    file.write(reinterpret_cast<const char *>(&num_channels), 2);   // NbrChannels      (2 bytes) : # of channels
    file.write(reinterpret_cast<const char *>(&sample_rate), 4);    // Frequency        (4 bytes) : Sample rate in hertz

    int byte_per_bloc = num_channels * (bits_per_sample / CHAR_BIT); // number of bytes in a single complete audio frame
    int byte_per_sec = sample_rate * byte_per_bloc; // # of bytes / second to read
    file.write(reinterpret_cast<const char *>(&byte_per_sec), 4);   // BytePerSec       (4 bytes) : # of bytes to read per second
    file.write(reinterpret_cast<const char *>(&byte_per_bloc), 2);  // BytePerBloc      (2 bytes) : # of bytes per bloc (a single complete audio frame (a single frame from all channels) )
    file.write(reinterpret_cast<const char *>(&bits_per_sample), 2);// BitsPerSample    (2 bytes) : # of bits per sample

    // Chunk containing the sampled data
    file.write("data", 4);                                          // DataBlocID       (4 bytes) : Identifier "data"
    file.write(reinterpret_cast<const char *>(&data_size), 4);      // DataSize         (4 bytes) : SampledData size

    file.close();
    std::cout << "WAV file created: " << filename << std::endl;


}

void appendWav(const std::string &filename, const std::vector<short> &samples) {
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary );
    if (!file) {
        std::cerr << "ERROR: Failed to open " << filename << " while appending" << std::endl;
        return; 
    }

    std::vector<char> buffer;

    // Get current file size and update it | FileSize: (4 bytes)
    uint32_t file_size; 
    file.seekg(4, std::ios_base::beg); // Skip "RIFF" Identifier
    buffer.resize(4);
    file.read(buffer.data(), 4);
    file_size = *reinterpret_cast<const uint32_t *>(buffer.data());

    uint32_t new_data_size = samples.size() * sizeof(short);   
    uint32_t new_file_size = file_size + new_data_size;
    file.seekp(4, std::ios_base::beg);
    file.write(reinterpret_cast<const char *>(&new_file_size), 4);

    // get num_channels from file
    uint16_t num_channels;
    file.seekg(22, std::ios_base::beg);
    buffer.resize(2);
    file.read(buffer.data(), 2);
    num_channels = *reinterpret_cast<const uint16_t *>(buffer.data());

    // get sample_rate from file
    uint32_t sample_rate;
    buffer.resize(4);
    file.read(buffer.data(), 4);
    sample_rate = *reinterpret_cast<const uint32_t *>(buffer.data());

    // rewrite datasize
    uint32_t data_size;
    file.seekg(40, std::ios_base::beg);
    buffer.resize(4);
    file.read(buffer.data(), 4);
    
    data_size = *reinterpret_cast<const uint32_t *>(buffer.data());
    uint32_t total_data_size = data_size + new_data_size;
    file.seekp(40, std::ios_base::beg);
    file.write(reinterpret_cast<const char *>(&total_data_size), 4);

    // skip old data & append new samples
    file.seekp(0, std::ios_base::end);
    file.write(reinterpret_cast<const char *>(samples.data()), new_data_size);

    file.close();
    std::cout << "WAV file appended: " << filename << std::endl;

}