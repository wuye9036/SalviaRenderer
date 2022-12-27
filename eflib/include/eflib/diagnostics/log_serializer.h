#pragma once

#include <eflib/io/stream.h>

#include <boost/tuple/tuple.hpp>

#include <fstream>
#include <ostream>
#include <vector>

namespace eflib {
/** Serializing the log
        And the format of log is��
                output	::= items
                items	::= {item|(indent items)}
                item	::= {content}item_splitter
                content ::= {string|(key keyval_splitter val)}
*/
class text_log_serializer {
public:
  text_log_serializer(std::ostream &str, const std::string &indent,
                      const std::string &item_splitter, const std::string &keyval_splitter)
      : ostr_(str), indent_(indent), item_splitter_(item_splitter),
        keyval_splitter_(keyval_splitter) {
    indent_stack_.push_back("");
  }

  void push_token_state() {
    state_stack_.push_back(std::make_tuple(indent_, item_splitter_, keyval_splitter_));
  }

  void pop_token_state() {
    auto [ident_, item_splitter_, keyval_splitter_] = state_stack_.back();
    state_stack_.pop_back();
  }

  void set_indent(const std::string indent) { indent_ = indent; }

  void set_item_splitter(const std::string item_splitter) { item_splitter_ = item_splitter; }

  void set_keyval_splitter(const std::string &kvsplitter) { keyval_splitter_ = kvsplitter; }

  ~text_log_serializer() {}

  void begin_log() { indent_stack_.push_back(indent_stack_.back() + indent_); }

  void end_log() { indent_stack_.pop_back(); }

  template <class T> void write(const std::string &key, const T &val) {
    ostr_ << indent_stack_.back() << key << keyval_splitter_ << val << item_splitter_;
  }

  template <class T> void write(const T &val) {
    ostr_ << indent_stack_.back() << val << item_splitter_;
  }

private:
  text_log_serializer &operator=(const text_log_serializer &rhs) = delete;
  text_log_serializer(const text_log_serializer &);

  std::string indent_;
  std::string item_splitter_;
  std::string keyval_splitter_;

  std::vector<std::tuple<std::string, std::string, std::string>> state_stack_;

  std::vector<std::string> indent_stack_;

  std::ostream &ostr_;
};
} // namespace eflib