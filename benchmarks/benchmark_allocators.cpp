#include <chrono>
#include <iostream>
#include <string>
class Stopwatch {
private:
  std::string m_name;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

public:
  Stopwatch(const std::string& name) : m_name(name) {
    m_start = std::chrono::high_resolution_clock::now();
  }

  ~Stopwatch() {
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();

    std::cout << m_name << " took: " << dur << "us\n";
  }
};
#ifndef ITERS
#define ITERS 100000
#endif

