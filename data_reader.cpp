#include <algorithm>
#include <limits>
#include <opencv4/opencv2/features2d.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <vector>

struct Stats {
  double min;
  double max;
  double median;
  double mean;
  double value;
};

int main(int argc, char **argv) {

  // Graph variable performance vs runtime (ms)

  // Get all nodes from a thread's file

  // for each vector, find the min, max, median, and mean of it

  // then for example, we know when ORDER = 3
  // the min, max, median, and mean runtime given randomized variables
  // and the relationship between it and other variables

  cv::FileStorage fs;

  std::string path = "./gen_times/";
  std::vector<std::string> filenames = {"audio_gen_times.yml",
                                        "depth_times.yml", "image_times.yml",
                                        "play_times.yml", "sine_times.yml"};

  using stats_vector = std::vector<std::vector<Stats>>;
  stats_vector order_stats(filenames.size());
  stats_vector sample_rate_stats(filenames.size());
  stats_vector block_size_stats(filenames.size());
  stats_vector disparity_stats(filenames.size());

  for (int i = 0; i < filenames.size(); i++) {
    fs.open(path + filenames.at(i), cv::FileStorage::READ);
    if (fs.isOpened()) {

      // Iterate over all entries
      for (cv::FileNodeIterator it = fs.root().begin(); it != fs.root().end();
           it++) {
        std::string key = (*it).name();
        cv::FileNode node = *it;

        std::vector<double> times;
        node >> times;

        std::string variable_name;
        double variable_value = -1;
        if (key.find("ORDER") != std::string::npos) {
          variable_name = "ORDER";
          std::string value = key.substr(5, key.length());
          std::replace(value.begin(), value.end(), '_', '.');
          variable_value = std::stod(value);
        } else if (key.find("SAMPLE_RATE") != std::string::npos) {
          variable_name = "SAMPLE_RATE";
          std::string value = key.substr(11, key.length());
          std::replace(value.begin(), value.end(), '_', '.');
          variable_value = std::stod(value);
        } else if (key.find("BLOCK_SIZE") != std::string::npos) {
          variable_name = "BLOCK_SIZE";
          std::string value = key.substr(10, key.length());
          std::replace(value.begin(), value.end(), '_', '.');
          variable_value = std::stod(value);
        } else if (key.find("NUM_DISPARITIES") != std::string::npos) {
          variable_name = "NUM_DISPARITIES";
          std::string value = key.substr(15, key.length());
          std::replace(value.begin(), value.end(), '_', '.');
          variable_value = std::stod(value);
        }

        if (variable_value == -1)
          continue;

        // Find the entry's statistics and store it in a struct
        double min = std::numeric_limits<double>::max();
        double max = 0;
        double median = 0;
        double mean = 0;
        double sum = 0;
        int len_times = times.size();
        if (len_times < 1) continue;
        for (int i = 0; i < len_times; i++) {
          double time = times.at(i);
          sum += time;
        }

        mean = sum / times.size();
        std::sort(times.begin(), times.end());

        if (len_times % 2 == 0) {
          median =
              (times.at((len_times - 1) / 2) + times.at(len_times / 2)) / 2.0;
        } else {
          median = times.at(len_times / 2);
        }
        min = times.at(0);
        max = times.at(times.size() - 1);

        // Add stats struct to respective array
        Stats current_stats = {min, max, median, mean, variable_value};
        if (variable_name == "ORDER")
          order_stats.at(i).push_back(current_stats);
        else if (variable_name == "SAMPLE_RATE")
          sample_rate_stats.at(i).push_back(current_stats);
        else if (variable_name == "BLOCK_SIZE")
          block_size_stats.at(i).push_back(current_stats);
        else
          disparity_stats.at(i).push_back(current_stats);
      }

      fs.release();
    }
  }

  for(int i = 0; i < filenames.size(); i++) {
    std::cout << filenames.at(i) << " runtime statistics ----------------------------------------" << std::endl;

    //-----------------------------------------------------------------------------//

    std::cout << "ORDER values" << std::endl;
    std::cout << "x_values = [";
    for (int n = 0; n < order_stats.at(i).size(); n++) {
      std::cout << order_stats.at(i).at(n).value << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_min = [";
    for (int n = 0; n < order_stats.at(i).size(); n++) {
      std::cout << order_stats.at(i).at(n).min << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_max = [";
    for (int n = 0; n < order_stats.at(i).size(); n++) {
      std::cout << order_stats.at(i).at(n).max << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_median = [";
    for (int n = 0; n < order_stats.at(i).size(); n++) {
      std::cout << order_stats.at(i).at(n).median << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_mean = [";
    for (int n = 0; n < order_stats.at(i).size(); n++) {
      std::cout << order_stats.at(i).at(n).mean << ", ";
    }
    std::cout << "]" << std::endl;

    //-------------------------------------------------------------------------------------------------------------//

    std::cout << "SAMPLE_RATE values" << std::endl;
    std::cout << "x_values = [";
    for (int n = 0; n < sample_rate_stats.at(i).size(); n++) {
      std::cout << sample_rate_stats.at(i).at(n).value << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_min = [";
    for (int n = 0; n < sample_rate_stats.at(i).size(); n++) {
      std::cout << sample_rate_stats.at(i).at(n).min << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_max = [";
    for (int n = 0; n < sample_rate_stats.at(i).size(); n++) {
      std::cout << sample_rate_stats.at(i).at(n).max << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_median = [";
    for (int n = 0; n < sample_rate_stats.at(i).size(); n++) {
      std::cout << sample_rate_stats.at(i).at(n).median << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_mean = [";
    for (int n = 0; n < sample_rate_stats.at(i).size(); n++) {
      std::cout << sample_rate_stats.at(i).at(n).mean << ", ";
    }
    std::cout << "]" << std::endl;

    //-------------------------------------------------------------------------------------------------------------//
    
    std::cout << "BLOCK_SIZE values" << std::endl;
    std::cout << "x_values = [";
    for (int n = 0; n < block_size_stats.at(i).size(); n++) {
      std::cout << block_size_stats.at(i).at(n).value << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_min = [";
    for (int n = 0; n < block_size_stats.at(i).size(); n++) {
      std::cout << block_size_stats.at(i).at(n).min << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_max = [";
    for (int n = 0; n < block_size_stats.at(i).size(); n++) {
      std::cout << block_size_stats.at(i).at(n).max << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_median = [";
    for (int n = 0; n < block_size_stats.at(i).size(); n++) {
      std::cout << block_size_stats.at(i).at(n).median << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_mean = [";
    for (int n = 0; n < block_size_stats.at(i).size(); n++) {
      std::cout << block_size_stats.at(i).at(n).mean << ", ";
    }
    std::cout << "]" << std::endl;

    //-------------------------------------------------------------------------------------------------------------//
    
    std::cout << "NUM_DISPARITY values" << std::endl;
    std::cout << "x_values = [";
    for (int n = 0; n < disparity_stats.at(i).size(); n++) {
      std::cout << disparity_stats.at(i).at(n).value << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_min = [";
    for (int n = 0; n < disparity_stats.at(i).size(); n++) {
      std::cout << disparity_stats.at(i).at(n).min << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_max = [";
    for (int n = 0; n < disparity_stats.at(i).size(); n++) {
      std::cout << disparity_stats.at(i).at(n).max << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_median = [";
    for (int n = 0; n < disparity_stats.at(i).size(); n++) {
      std::cout << disparity_stats.at(i).at(n).median << ", ";
    }
    std::cout << "]" << std::endl;

    std::cout << "y_mean = [";
    for (int n = 0; n < disparity_stats.at(i).size(); n++) {
      std::cout << disparity_stats.at(i).at(n).mean << ", ";
    }
    std::cout << "]" << std::endl;

  }

  return 0;
}
