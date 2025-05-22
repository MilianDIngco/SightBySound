#include "opencv2/core/persistence.hpp"
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/features2d.hpp>
#include <string>
#include <random>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <sys/wait.h>

using hr_clock = std::chrono::high_resolution_clock;
using milliseconds = std::chrono::milliseconds;

struct Field {
  std::string name;
  float lower_bound;
  float upper_bound;
  float increment; 
  float value;
  int multiple;

  Field() : name(""), lower_bound(0), upper_bound(0), increment(0) {}
  Field(std::string name, float lower, float upper, float increment) : name(name), lower_bound(lower), upper_bound(upper), increment(increment) {
    multiple = (int) (upper_bound - lower_bound) / increment;
  }

  void set_value(float value) {this->value = value;}
  float get_value() {return this->value;}
};

/**
 * Takes 
 * variable name, number of samples
 * */
int main(int argc, char** argv) {

  if (argc < 3) {
    std::cerr << "ERROR: Not enough parameters" << std::endl;
    return 1;
  }

  std::cout << "Seeding random generator" << std::endl;
  hr_clock::time_point now = hr_clock::now();
  milliseconds time = std::chrono::duration_cast<milliseconds>(now.time_since_epoch());

  std::default_random_engine gen(time.count());
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  auto gen_random = std::bind(dist, gen);

  std::cout << "Opening settings file" << std::endl;
  cv::FileStorage fs("cv_settings.yml", cv::FileStorage::WRITE);

  /*
   * Independent Variables: 
   * -Order
   * -Sample_rate
   * -Block size
   * -Num disparities
   *
   * Dependent Variables:
   * Time from capturing picture to playing that pictures audio
   *  = sum of avg time of each thread
   */

  Field order("ORDER", 3, 5, 1);
  Field sample_rate("SAMPLE_RATE", 11000, 44000, 1000);
  Field block_size("BLOCK_SIZE", 5, 35, 2);
  Field num_disp("NUM_DISPARITIES", 16, 96, 16);

  std::unordered_map<std::string, Field> var_map;
  var_map.emplace("ORDER", order);
  var_map.emplace("SAMPLE_RATE", sample_rate);
  var_map.emplace("BLOCK_SIZE", block_size);
  var_map.emplace("NUM_DISPARITIES", num_disp);

  std::string test_var = argv[1];
  int n_samples = std::stoi(argv[2]);

  Field& test = var_map[test_var];
  int range = (int) (test.upper_bound - test.lower_bound) / test.increment;
  
  for (int i = 0; i <= range; i++) 
  {
    // Set testing value   
    test.set_value(test.lower_bound + (test.increment * i));
    std::cout << test_var << " was set to " << test.get_value() << std::endl;

    for (int n = 0; n < n_samples; n++) 
    {
      // Reopen if it was closed
      if (!fs.isOpened())
        fs.open("cv_settings.yml", cv::FileStorage::WRITE);

      // Set main value
      fs << "TEST_VAR" << test_var;
      fs << "TEST_VALUE" << test.get_value();
      fs << test_var << test.get_value();
      // Set random values for others
      for (auto& [key, field] : var_map) 
      {
        if (key == test_var) continue;

        float random = gen_random();
        int multiple = (int) ((float) field.multiple * random);

        field.set_value(field.lower_bound + field.increment * multiple);

        fs << key << field.get_value();

        std::cout << key << " was set to " << field.get_value() << std::endl;
      }
      if (fs.isOpened())
        fs.release();

      // Run to test performance
      pid_t pid = fork();
      if (pid == 0) { // Child process
        execl("./main", "./main", (char*)NULL);
        perror("execl");
        exit(1);
      } else if(pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        std::cout << "Child exited with status " << status << std::endl;
      } else {
        perror("fork");
        exit(1);
      }
    }

  }

  fs.release();

  return 0;
}
