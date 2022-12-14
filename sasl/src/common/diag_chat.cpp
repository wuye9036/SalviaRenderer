#include <sasl/common/diag_chat.h>

#include <sasl/common/diag_item.h>
#include <eflib/diagnostics/assert.h>
#include <vector>

using std::shared_ptr;
using std::string_view;
using std::unique_ptr;
using std::make_unique;
using std::vector;

namespace sasl::common {

shared_ptr<diag_chat> diag_chat::create() { return std::make_shared<diag_chat>(); }

void diag_chat::add_report_raised_handler(report_handler_fn const &handler) { handlers_.push_back(handler); }

void diag_chat::report_args(diag_template tmpl, std::string_view file_name, code_span span, fmt::format_args args) {
  std::string resolved_diag_message = fmt::vformat(tmpl.content, args);
  diags_.emplace_back(tmpl, file_name, span, std::move(resolved_diag_message));
}

void diag_chat::report(diag_template tmpl, std::string_view file_name, code_span span) {
  diags_.emplace_back(tmpl, file_name, span, std::string{tmpl.content});
}

void diag_chat::report_args(diag_template tmpl, token_t token_beg, token_t token_end, fmt::format_args args) {
  report_args(tmpl, token_beg.file_name, sasl::common::merge(token_beg.span, token_end.span), args);
}

void diag_chat::report(diag_template tmpl, token_t token_beg, token_t token_end) {
  report(tmpl, token_beg.file_name, sasl::common::merge(token_beg.span, token_end.span));
}

diag_chat *diag_chat::merge(diag_chat *dest, diag_chat *src, bool trigger_callback) {
  dest->diags_.insert(dest->diags_.end(), src->diags_.begin(), src->diags_.end());

  if (trigger_callback) {
    for (auto& diag : src->diags_) {
      for (auto handler : dest->handlers_) {
        handler(dest, &diag);
      }
    }
  }

  src->diags_.clear();
  return dest;
}

void diag_chat::restore() {
  diags_.erase(diags_.begin() + cursors_.back());
  cursors_.pop_back();
}

void diag_chat::save() { cursors_.push_back(diags_.size()); }

diag_chat::~diag_chat() {}

void diag_chat::clear() {
  diags_.clear();
}

size_t error_count(diag_chat const* chat, bool warning_as_error) {
  size_t count = 0;

  for(auto const& diag: chat->items()) {
	if (diag.level() == diag_levels::error || diag.level() == diag_levels::fatal_error) {
      ++count;
    } else if (warning_as_error && diag.level() == diag_levels::warning) {
      ++count;
    }
  }

  return count;
}

} // namespace sasl::common