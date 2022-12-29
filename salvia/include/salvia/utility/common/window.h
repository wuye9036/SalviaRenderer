#pragma once

#include <salvia/utility/api_symbols.h>

#include <boost/signals2.hpp>

#include <any>
#include <functional>

namespace salvia::utility {

typedef std::function<void()> idle_handler_t;
typedef std::function<void()> draw_handler_t;
typedef std::function<void()> create_handler_t;

class window {
public:
  virtual void show() = 0;

  /** Event handlers
  @{ */
  virtual void set_idle_handler(idle_handler_t const &handler) = 0;
  virtual void set_draw_handler(draw_handler_t const &handler) = 0;
  virtual void set_create_handler(create_handler_t const &handler) = 0;
  /** @} */

  /** Properties @{ */
  virtual void *view_handle_as_void() = 0;
  virtual std::any view_handle() = 0;
  virtual void set_title(std::string const &) = 0;
  /** @} */

  virtual void refresh() = 0;

  virtual ~window() = default;
};

}