#ifndef FUNCTION_TIMER_H
#define FUNCTION_TIMER_H

#include <chrono>
#include <mutex>
#include <thread>

using chrono_ms = std::chrono::milliseconds;
using hr_clock = std::chrono::high_resolution_clock;

class FunctionTimer {
    private:
        std::mutex active_mutex;
        std::mutex result_mutex;
        bool active;

    public:
        template<typename Func, typename Return, typename... Args>
        void set_timeout(Func function, int timer_ms, Return& output, Return& default_output, Args&&... args) {
            {
                std::lock_guard<std::mutex> lock(active_mutex);
                active = true;
            }
        
            chrono_ms timeout(timer_ms);
            auto start = hr_clock::now();
            Return result = default_output;
        
            std::thread t([&]() {
                Return temp = function(std::forward<Args>(args)...);
                {
                    std::lock_guard<std::mutex> lock(result_mutex);
                    result = temp;
                }
                {
                    std::lock_guard<std::mutex> lock(active_mutex);
                    active = false;
                }
            });
        
            t.detach();
        
            while (true) {
                {
                    std::lock_guard<std::mutex> lock(active_mutex);
                    if (!active) break;
                }
        
                if (std::chrono::duration_cast<chrono_ms>(hr_clock::now() - start) >= timeout)
                    break;
        
                std::this_thread::sleep_for(chrono_ms(1));
            }
        
            {
                std::lock_guard<std::mutex> lock(active_mutex);
                if (active) {
                    output = default_output;
                } else {
                    std::lock_guard<std::mutex> result_lock(result_mutex);
                    output = result;
                }
            }
        }
};

#endif // FUNCTION_TIMER_H