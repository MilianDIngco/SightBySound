#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <unordered_map>
#include <variant>

class Settings {
  public:
    int order;
    double min_freq;
    double max_freq;
    double duration;
    int sample_rate;
    double volume;
    bool save_img;
    std::string save_img_path;
    int n_runs;
    double fade_percent;
    std::string camera_calibration_path;
    std::string right_camera_path;
    std::string left_camera_path;
    int block_size;
    int num_disparities;
    int prefilter_cap;
    int min_disparity;
    int texture_threshold;
    int uniqueness_ratio;
    int speckle_window_size;
    int speckle_range;
    int disp12maxdiff;
    double prestereo_scale;
    int max_buffer;
    bool use_internearest;
    bool use_interarea;
    bool debug_print;

    using ValueType = std::variant<int, double, std::string, bool>;
    std::unordered_map<std::string, ValueType> settings_map;

    Settings(const std::string &settings_path);
  
    template<typename T>
    T get(const std::string name) {
      return std::get<T>(settings_map.at(name));
    }

    void loadFile(const std::string &filepath);
};

#endif // !SETTINGS_H
