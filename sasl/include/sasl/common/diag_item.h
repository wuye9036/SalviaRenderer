#pragma once

#include <sasl/common/common_fwd.h>

#include <sasl/common/token.h>

#include <boost/format.hpp>
#include <eflib/platform/boost_begin.h>
#include <eflib/platform/boost_end.h>

#include <memory>
#include <string>

namespace sasl::common {

enum diag_levels {
  dl_fatal_error, // Fatal Error
  dl_error,       // Error
  dl_warning,     // Warning
  dl_info,        // Information
  dl_text         // Text.
};

class diag_template;

class diag_data {
public:
  virtual void apply(boost::format &fmt) = 0;
  virtual ~diag_data() {}
  virtual void release() { delete this; }
};

template <typename T> class diag_data_impl : public diag_data {
public:
  diag_data_impl(T const &v) : value(v) {}
  void apply(boost::format &fmt) { fmt % value; }

private:
  diag_data_impl(diag_data_impl<T> const &);
  diag_data_impl<T> &operator=(diag_data_impl<T> const &);
  T value;
};

class diag_item {
public:
  diag_item(diag_template const *tmpl);
  ~diag_item();

  template <typename T> diag_item &operator%(T const &v) {
    fmt_params.push_back(new diag_data_impl<T>(v));
    return *this;
  }

  diag_item &eval();
  diag_item &file(std::string_view f);
  diag_item &span(token_t const &beg, token_t const &end);
  diag_item &span(code_span const &s);

  bool is_template(diag_template const &v) const;
  diag_levels level() const;
  std::string_view str() const;
  code_span span() const;
  std::string_view file() const;
  size_t id() const;

  void release();

private:
  boost::format &formatter();
  boost::format const &formatter() const;

  std::string_view item_file;
  code_span item_span;
  diag_template const *tmpl;
  std::unique_ptr<boost::format> fmt;
  mutable std::vector<diag_data *> fmt_params;
};

class diag_template {
public:
  diag_template(size_t uid, diag_levels lvl, std::string const &str);
  diag_template(diag_levels lvl, std::string const &str);

  std::string const &template_str() const;
  diag_levels level() const;
  size_t id() const;

  static size_t automatic_id();

private:
  size_t uid;
  diag_levels lvl;
  std::string tmpl;
};

}
