#include "functiontimer.hpp"
#include <chrono>
#include <iostream>

FunctionTimer::FunctionTimer() {
  start = std::chrono::high_resolution_clock::now();
  end = std::chrono::high_resolution_clock::now();
  runs = 0;

  min = std::chrono::seconds::max();
  max = std::chrono::seconds::zero();
}

void FunctionTimer::start_clock() {
  start = std::chrono::high_resolution_clock::now();
}

void FunctionTimer::stop_clock() {
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  total_elapsed += elapsed;
  runs++;
  if (elapsed > max)
    max = elapsed;
  if (elapsed < min)
    min = elapsed;
}

void FunctionTimer::print_average(std::string name) {
  std::chrono::duration<double> average_elapsed = total_elapsed / runs;
  std::cout << name << " has run " << runs << " times" << std::endl;
  std::cout << "Average time of : " << average_elapsed.count() << " seconds per run" << std::endl;
  std::cout << "Max runtime was : " << max.count() << " seconds" << std::endl;
  std::cout << "Min runtime was : " << min.count() << " seconds" << std::endl;
}
