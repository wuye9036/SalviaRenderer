#include <eflib/platform/stdint.h>
#include <salvia/utility/common/timer.h>

namespace salvia::utility {

timer::timer() {
  restart();
}

void timer::restart() {
  start_time_ = current_time();
}

double timer::elapsed() const {
  seconds sec = current_time() - start_time_;
  return sec.count();
}

timer::time_point timer::current_time() {
  return clock_type::now();
}

fps_counter::fps_counter(float interval)
  : elapsed_frame_(0)
  , elapsed_seconds_(0)
  , interval_(interval) {
}

bool fps_counter::on_frame(float& fps) {
  elapsed_seconds_ += static_cast<float>(timer_.elapsed());
  ++elapsed_frame_;
  timer_.restart();
  if (elapsed_seconds_ >= interval_) {
    fps = static_cast<float>(elapsed_frame_) / elapsed_seconds_;
    elapsed_seconds_ = 0;
    elapsed_frame_ = 0;
    return true;
  }

  return false;
}

}  // namespace salvia::utility