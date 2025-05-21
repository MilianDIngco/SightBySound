#ifndef FUNCTION_TIMER_H
#define FUNCTION_TIMER_H

#include <string>
#include <chrono>

class FunctionTimer {
  public:
    FunctionTimer();
    void start_clock();
    void stop_clock();
    void print_average(std::string name);
    std::chrono::duration<double> get_average();
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> end;
    std::chrono::duration<double> total_elapsed;
    std::chrono::duration<double> max;
    std::chrono::duration<double> min;
    int runs;
};

#endif // !FUNCTION_TIMER_H
