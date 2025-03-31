#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <functional>  // for std::invoke
#include <utility>     // for std::forward

// Template to measure the execution time of a void function with no arguments
template <typename Func>
double measureExecutionTime(Func&& func) {
    // Get the start time
    auto start = std::chrono::high_resolution_clock::now();

    // Call the function
    std::invoke(std::forward<Func>(func));

    // Get the end time
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration<double>(end - start);

    return duration.count();
}

#endif  // TIMER_H
