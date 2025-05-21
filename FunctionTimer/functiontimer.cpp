#include "functiontimer.hpp"
#include <chrono>
#include <iostream>

FunctionTimer::FunctionTimer() {
  start = std::chrono::high_resolution_clock::now();
  end = std::chrono::high_resolution_clock::now();
  total_elapsed = std::chrono::duration<double>::zero();
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
  std::chrono::duration<double> total_wo_max = total_elapsed - max;
  std::chrono::duration<double> average_wo_max = total_wo_max / runs;

  std::cout << name << " run " << runs << " times" << std::endl
            << name << " Average time of : " << average_elapsed.count()
            << " seconds per run" << std::endl
            << name << " Max runtime was : " << max.count() << " seconds"
            << std::endl
            << name << " Min runtime was : " << min.count() << " seconds"
            << std::endl
            << name
            << " Average time excluding max : " << average_wo_max.count()
            << " seconds" << std::endl;
}

std::chrono::duration<double> FunctionTimer::get_average() {
  std::chrono::duration<double> average_elapsed = total_elapsed / runs;
  std::chrono::duration<double> total_wo_max = total_elapsed - max;
  std::chrono::duration<double> average_wo_max = total_wo_max / runs;

  return average_wo_max;
}
