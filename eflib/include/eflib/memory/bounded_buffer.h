#pragma once

#include <boost/circular_buffer.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace eflib {
template <class T> class bounded_buffer {
public:
  typedef boost::circular_buffer<T> container_type;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::value_type value_type;

  explicit bounded_buffer(size_type capacity) : m_unread(0), m_container(capacity) {}

  template <typename U> void push_front(U &&item) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_not_full.wait(lock, [this]() -> bool { return this->is_not_full(); });
    m_container.push_front(std::forward<U>(item));
    ++m_unread;
    lock.unlock();
    m_not_empty.notify_one();
  }

  void pop_back(value_type *pItem) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_not_empty.wait(lock, [this]() -> bool { return this->is_not_empty(); });
    *pItem = m_container[--m_unread];
    lock.unlock();
    m_not_full.notify_one();
  }

private:
  bounded_buffer(const bounded_buffer &) = delete;            // Disabled copy constructor
  bounded_buffer &operator=(const bounded_buffer &) = delete; // Disabled assign operator

  bool is_not_empty() const { return m_unread > 0; }
  bool is_not_full() const { return m_unread < m_container.capacity(); }

  size_type m_unread;
  container_type m_container;
  std::mutex m_mutex;
  std::condition_variable m_not_empty;
  std::condition_variable m_not_full;
};
} // namespace eflib
