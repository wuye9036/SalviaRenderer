#pragma once

#include <salvia/utility/api_symbols.h>

#include <chrono>

namespace salvia::utility {

class SALVIA_UTILITY_API timer {
public:
  typedef std::chrono::duration<double> seconds;
  typedef std::chrono::high_resolution_clock clock_type;
  typedef clock_type::time_point time_point;

  timer();
  void restart();
  [[nodiscard]] double elapsed() const;
  static time_point current_time();

private:
  clock_type::time_point start_time_;
};

class SALVIA_UTILITY_API fps_counter {
public:
  explicit fps_counter(float interval);
  bool on_frame(float& fps);

private:
  timer timer_;
  uint32_t elapsed_frame_;
  float elapsed_seconds_;
  float interval_;
};

}  // namespace salvia::utility