#include "../include/settings.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

Settings::Settings(const std::string settings_path) {
  this->loadFile(settings_path);
}

void Settings::loadFile(std::string filepath) {
  std::ifstream settings(filepath);

  if (!settings.is_open()) {
    std::cerr << "ERROR: Failed to load settings from [" << filepath << "]" << std::endl;
    return;
  }

  std::string line;
  while (std::getline(settings, line)) {
    std::stringstream s(line);

    // Grab variable type
    char type;
    s >> type;
    type = std::tolower(type);

    // Grab variable name
    std::string name;
    s >> name;

    // Grab variable value
    s.ignore(1);
    std::string value;
    std::getline(s, value);

    switch (type) {
      case 'i':
        Settings::settings_map[name] = std::stoi(value);
        break;
      case 'd':
        Settings::settings_map[name] = std::stod(value);
        break;
      case 's':
        Settings::settings_map[name] = value;
        break;
      case 'b':
        Settings::settings_map[name] = (bool)std::stoi(value);
        break;
      default:
        std::cout << "ERROR: Setting type not defined";
        return;
    }
  }

  this->order = this->get<int>("order");
  this->min_freq = this->get<double>("min_freq");
  this->max_freq = this->get<double>("max_freq");
  this->duration = this->get<double>("duration");
  this->sample_rate = this->get<int>("sample_rate");
  this->volume = this->get<double>("volume");
  this->save_img = this->get<bool>("save_img");
  this->n_runs = this->get<int>("n_runs");
  this->fade_percent = this->get<double>("fade_percent");
  this->camera_calibration_path = this->get<std::string>("camera_calibration_path");
  this->right_camera_path = this->get<std::string>("right_camera_path");
  this->left_camera_path = this->get<std::string>("left_camera_path");
  this->block_size = this->get<int>("block_size");
  this->num_disparities = this->get<int>("num_disparities");
  this->prefilter_cap = this->get<int>("prefilter_cap");
  this->min_disparity = this->get<int>("min_disparity");
  this->texture_threshold = this->get<int>("texture_threshold");
  this->uniqueness_ratio = this->get<int>("uniqueness_ratio");
  this->speckle_window_size = this->get<int>("speckle_window_size");
  this->speckle_range = this->get<int>("speckle_range");
  this->disp12maxdiff = this->get<int>("disp12maxdiff");
  this->prestereo_scale = this->get<double>("prestereo_scale");
  this->max_buffer = this->get<int>("max_buffer");
  this->use_internearest = this->get<bool>("use_internearest");
  this->use_interarea = this->get<bool>("use_interarea");

}

