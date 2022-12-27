#pragma once

#ifndef SALVIAU_COMMON_TIMER_H
#define SALVIAU_COMMON_TIMER_H

#include <salviau/include/salviau_forward.h>

#include <chrono>

BEGIN_NS_SALVIAU();

class SALVIAU_API timer {
public:
  typedef std::chrono::duration<double> seconds;
  typedef std::chrono::high_resolution_clock clock_type;
  typedef clock_type::time_point time_point;

  timer();
  void restart();
  double elapsed() const;
  time_point current_time() const;

private:
  clock_type::time_point start_time_;
};

class SALVIAU_API fps_counter {
public:
  fps_counter(float interval);
  bool on_frame(float &fps);

private:
  timer timer_;
  uint32_t elapsed_frame_;
  float elapsed_seconds_;
  float fps_;
  float interval_;
};

END_NS_SALVIAU();

#endif // _TIMER_H
