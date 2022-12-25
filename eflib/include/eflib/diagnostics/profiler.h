#pragma once

#include <eflib/platform/typedefs.h>

#include <boost/property_tree/ptree.hpp>

#include <chrono>
#include <string>
#include <vector>

namespace eflib {
class profiling_item {
public:
  typedef std::chrono::high_resolution_clock clock;

  explicit profiling_item(profiling_item *parent);
  profiling_item(profiling_item const &) = delete;
  profiling_item(profiling_item &&) = default;
  ~profiling_item();

  void start(clock::time_point start_time);
  void end(clock::time_point end_time);

  double duration() const;
  double children_duration() const;
  double exclusive_duration() const;

  bool try_merge(profiling_item *rhs);

  size_t tag;
  std::string name;

  std::vector<std::unique_ptr<profiling_item>> children;
  profiling_item *parent;

private:
  clock::time_point start_time_;
  double duration_;
};

class profiler {
public:
  profiler();

  void start(std::string const &, size_t tag);
  void end(std::string const &);

  void merge_items();

  profiling_item const *root() const noexcept;
  profiling_item const *current() const noexcept;

private:
  profiling_item root_;
  profiling_item *current_;
};

class profiling_scope {
public:
  profiling_scope(profiler *prof, std::string const &name, size_t tag = 0);
  ~profiling_scope();

private:
  profiling_scope(const profiling_scope &) = delete;
  profiling_scope &operator=(const profiling_scope &) = delete;

  profiler *prof;
  profiling_item const *current_checkpoint;
  std::string name;
};

void print_profiler(profiler const *prof, size_t max_level);
boost::property_tree::ptree make_ptree(profiler const *prof, size_t max_level);
} // namespace eflib
