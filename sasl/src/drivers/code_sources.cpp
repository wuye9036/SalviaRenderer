#include <sasl/drivers/code_sources.h>

#include <sasl/common/diag_chat.h>
#include <sasl/parser/diags.h>

#include <eflib/diagnostics/assert.h>

using namespace sasl::parser::diags;
using namespace sasl::common;

using boost::wave::preprocess_exception;

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ios_base;
using std::istream_iterator;
using std::shared_ptr;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

namespace sasl::drivers {

unordered_map<void*, compiler_code_source*> compiler_code_source::ctxt_to_source;

bool compiler_code_source::set_code(string const& code) {
  this->code = code;
  fixes_file_end_with_newline(this->code);
  this->filename = "<In Memory>";
  return process();
}

bool compiler_code_source::set_file(string const& file_name) {
  ifstream in(file_name.c_str(), ios_base::in);
  if (!in) {
    return false;
  } else {
    in.unsetf(ios::skipws);
    code.assign(istream_iterator<char>(in), istream_iterator<char>());
    fixes_file_end_with_newline(code);
    filename = file_name;
  }
  in.close();

  return process();
}

bool compiler_code_source::eof() {
  try {
    return (next_it == wave_ctxt_wrapper->get_wave_ctxt()->end());
  } catch (preprocess_exception& e) {
    report_wave_pp_exception(diags, e);
    next_it = wave_ctxt_wrapper->get_wave_ctxt()->end();
    return false;
  }
}

string_view compiler_code_source::next() {
  cur_it = next_it;

  try {
    ++next_it;
  } catch (preprocess_exception& e) {
    report_wave_pp_exception(diags, e);
    err_token = to_string(cur_it->get_value());
    next_it = wave_ctxt_wrapper->get_wave_ctxt()->end();
  } catch (wave_reported_fatal_error&) {
    // Error has been reported to diag_chat.
    err_token = to_string(cur_it->get_value());
    next_it = wave_ctxt_wrapper->get_wave_ctxt()->end();
    is_failed = true;
  }

  return to_string_view(cur_it->get_value());
}

string_view compiler_code_source::error() {
  if (err_token.empty()) {
    err_token = to_string(cur_it->get_value());
  }
  return err_token;
}

void compiler_code_source::report_wave_pp_exception(diag_chat* diags, preprocess_exception& e) {
  diag_template const* tmpl = nullptr;
  switch (e.get_errorcode()) {
  case preprocess_exception::no_error: break;
  case preprocess_exception::last_line_not_terminated: tmpl = &boost_wave_exception_warning; break;
  case preprocess_exception::ill_formed_directive:
    tmpl = &boost_wave_exception_fatal_error;
    is_failed = true;
    break;
  default:
    is_failed = true;
    ef_unimplemented();
    break;
  }

  string file_name;
  if (tmpl) {
    try {
      file_name = to_string(cur_it->get_position().get_file());
    } catch (...) {
      file_name = wave_ctxt_wrapper->get_wave_ctxt()->get_current_filename();
    }
  }

  diags->report(
      *tmpl, file_name, current_span(), preprocess_exception::error_text(e.get_errorcode()));
}

string_view compiler_code_source::file_name() const {
  ef_verify(is_failed || cur_it != wave_ctxt_wrapper->get_wave_ctxt()->end());

  filename = to_string(cur_it->get_position().get_file());
  return filename;
}

size_t compiler_code_source::column() const {
  ef_verify(is_failed || cur_it != wave_ctxt_wrapper->get_wave_ctxt()->end());
  return cur_it->get_position().get_column();
}

size_t compiler_code_source::line() const {
  ef_verify(is_failed || cur_it != wave_ctxt_wrapper->get_wave_ctxt()->end());
  return cur_it->get_position().get_line();
}

void compiler_code_source::update_position(string_view /*lit*/) {
  // Do nothing.
  return;
}

bool compiler_code_source::process() {
  wave_ctxt_wrapper.reset(
      new wave_context_wrapper(this, new wave_context(code.begin(), code.end(), filename.c_str())));

  size_t lang_flag = boost::wave::support_cpp;
  lang_flag &= ~(boost::wave::support_option_emit_line_directives);
  lang_flag &= ~(boost::wave::support_option_single_line);
  lang_flag &= ~(boost::wave::support_option_emit_pragma_directives);
  wave_ctxt_wrapper->get_wave_ctxt()->set_language(
      static_cast<boost::wave::language_support>(lang_flag));

  cur_it = wave_ctxt_wrapper->get_wave_ctxt()->begin();
  next_it = wave_ctxt_wrapper->get_wave_ctxt()->begin();
  is_failed = false;

  return true;
}

void compiler_code_source::set_diag_chat(sasl::common::diag_chat* diags) {
  this->diags = diags;
}

compiler_code_source::compiler_code_source() : diags(nullptr) {
}

compiler_code_source::~compiler_code_source() {
}

code_span compiler_code_source::current_span() const {
  auto const& pos = cur_it->get_position();
  return inline_code_span(pos.get_line(), pos.get_column(), pos.get_column() + 1);
}

void compiler_code_source::add_virtual_file(string const& file_name,
                                            string const& content,
                                            bool high_priority) {
  virtual_files[file_name] = make_pair(high_priority, content);
}

void compiler_code_source::set_include_handler(include_handler_fn handler) {
  do_include = handler;
}

compiler_code_source* compiler_code_source::get_code_source(void* ctxt) {
  if (ctxt) {
    auto it = ctxt_to_source.find(ctxt);
    if (it != ctxt_to_source.end()) {
      return it->second;
    }
  }

  return nullptr;
}

bool compiler_code_source::failed() {
  return is_failed;
}

bool compiler_code_source::add_sys_include_path(string const& path) {
  wave_context* wave_ctxt = wave_ctxt_wrapper->get_wave_ctxt();
  ef_verify(wave_ctxt);
  return wave_ctxt->add_sysinclude_path(path.c_str());
}

bool compiler_code_source::add_sys_include_path(vector<string> const& paths) {
  bool ret = true;
  for (size_t i = 0; i < paths.size(); ++i) {
    bool path_ret = add_sys_include_path(paths[i]);
    ret = ret && path_ret;
  }
  return ret;
}

bool compiler_code_source::add_include_path(string const& path) {
  wave_context* wave_ctxt = wave_ctxt_wrapper->get_wave_ctxt();
  ef_verify(wave_ctxt);
  return wave_ctxt->add_include_path(path.c_str());
}

bool compiler_code_source::add_include_path(vector<string> const& paths) {
  bool ret = true;
  for (size_t i = 0; i < paths.size(); ++i) {
    ret = add_include_path(paths[i]) && ret;
  }
  return ret;
}

bool compiler_code_source::add_macro(string const& macro_def, bool predef) {
  wave_context* wave_ctxt = wave_ctxt_wrapper->get_wave_ctxt();
  ef_verify(wave_ctxt);
  return wave_ctxt->add_macro_definition(macro_def, predef);
}

bool compiler_code_source::add_macro(vector<string> const& macros, bool predef) {
  bool ret = true;
  for (size_t i = 0; i < macros.size(); ++i) {
    ret = add_macro(macros[i], predef) && ret;
  }
  return ret;
}

bool compiler_code_source::clear_macros() {
  wave_context* wave_ctxt = wave_ctxt_wrapper->get_wave_ctxt();
  ef_verify(wave_ctxt);
  wave_ctxt->reset_macro_definitions();
  return true;
}

bool compiler_code_source::remove_macro(string const& macro_def) {
  wave_context* wave_ctxt = wave_ctxt_wrapper->get_wave_ctxt();
  ef_verify(wave_ctxt);
  return wave_ctxt->remove_macro_definition(macro_def);
}

bool compiler_code_source::remove_macro(vector<string> const& macros) {
  bool ret = true;
  for (size_t i = 0; i < macros.size(); ++i) {
    ret = remove_macro(macros[i]) && ret;
  }
  return ret;
}

void report_load_file_failed(void* ctxt, string const& name, bool /*is_system*/) {
  compiler_code_source* code_src = compiler_code_source::get_code_source(ctxt);
  code_src->diags->report(
      cannot_open_include_file, code_src->file_name(), code_src->current_span(), name);
}

void load_virtual_file(bool& is_succeed,
                       bool& is_exclusive,
                       string& content,
                       void* ctxt,
                       string const& name,
                       bool is_system,
                       bool before_file_load) {
  is_exclusive = false;
  is_succeed = false;

  compiler_code_source* code_src = compiler_code_source::get_code_source(ctxt);
  ef_verify(code_src);
  if (code_src->do_include) {
    string native_name;
    is_exclusive = true;
    is_succeed = code_src->do_include(content, native_name, name, is_system, false);
    fixes_file_end_with_newline(content);
    return;
  }

  if (!is_system) {
    return;
  }

  auto it = code_src->virtual_files.find(name);
  if (it == code_src->virtual_files.end()) {
    return;
  }

  bool hi_prior = it->second.first;
  if (hi_prior == before_file_load) {
    content = it->second.second;
    fixes_file_end_with_newline(content);
    is_succeed = true;
    return;
  }

  return;
}

void check_file(bool& is_succeed,
                bool& is_exclusive,
                string& native_name,
                void* ctxt,
                string const& name,
                bool is_system,
                bool is_before_include) {
  is_exclusive = false;
  is_succeed = false;

  compiler_code_source* code_src = compiler_code_source::get_code_source(ctxt);
  ef_verify(code_src);

  // If inc handler is enabled, it is exclusive.
  if (code_src->do_include) {
    string content;
    is_exclusive = true;
    is_succeed = code_src->do_include(content, native_name, name, is_system, true);
    return;
  }

  auto it = code_src->virtual_files.find(name);
  if (it == code_src->virtual_files.end()) {
    return;
  }

  bool hi_prior = it->second.first;
  if (hi_prior == is_before_include) {
    native_name = name;
    is_succeed = true;
    return;
  }

  return;
}

wave_context_wrapper::wave_context_wrapper(compiler_code_source* src, wave_context* ctx) {
  if (wave_ctxt) {
    compiler_code_source::ctxt_to_source.erase(wave_ctxt.get());
  }
  compiler_code_source::ctxt_to_source[ctx] = src;
  wave_ctxt.reset(ctx);
}

wave_context_wrapper::~wave_context_wrapper() {
  if (wave_ctxt) {
    compiler_code_source::ctxt_to_source.erase(wave_ctxt.get());
  }
  wave_ctxt.reset(nullptr);
}

wave_context* wave_context_wrapper::get_wave_ctxt() const {
  return wave_ctxt.get();
}

void fixes_file_end_with_newline(string& content) {
  if (content.empty()) {
    return;
  }

  char end_ch = *content.rbegin();
  if (end_ch == '\r' || end_ch == '\n') {
    return;
  }

#if defined(EFLIB_WINDOWS)
  content.append("\r\n");
#elif defined(EFLIB_OSX)
  content.append("\r");
#elif defined(EFLIB_UNIX) || defined(EFLIB_LINUX)
  content.append("\n");
#endif
}

}  // namespace sasl::drivers
