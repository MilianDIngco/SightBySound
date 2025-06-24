#include "sightbysound.hpp"
#include <string>

using vec2D = std::vector<std::vector<int>>;
using vec1D = std::vector<int>;
int main(int argc, char** argv) {
  std::string settings_filepath = "./settings/laptop_settings.txt";
  SightBySound sbs(settings_filepath);
  sbs.run();
  std::cout << "Finished" << std::endl;
  return 0;
}
