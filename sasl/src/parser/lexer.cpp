#include <sasl/parser/lexer.h>

#include <sasl/common/token.h>

#include <eflib/diagnostics/assert.h>
#include <eflib/utility/hash.h>

#include <boost/spirit/include/lex.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>

#include <unordered_set>

namespace splex = boost::spirit::lex;

using sasl::common::lex_context;
using sasl::common::token;

using std::make_shared;
using std::shared_ptr;
using std::unordered_map;
using std::unordered_set;

using std::string;
using std::vector;

namespace sasl::parser {

struct pair_hash {
  template <typename T, typename U>
  size_t operator()(std::pair<T, U> const& v) const noexcept {
    using std::hash;
    return hash<T>()(v.first) ^ hash<U>()(v.second);
  }
};

class shared_data {
public:
  shared_data() : attrs(nullptr) {}
  std::unordered_map<std::pair<size_t, std::string>, std::string, pair_hash> state_translations;
  unordered_set<std::string> skippers;
  unordered_set<std::string> init_states;
  std::vector<token>* attrs;
  shared_ptr<lex_context> ctxt;
};

class attribute_processor {
public:
  attribute_processor() : data{make_shared<shared_data>()} {}
  attribute_processor(attribute_processor const& rhs) : data(rhs.data) {}
  attribute_processor& operator=(attribute_processor const&) = delete;

  void output(std::vector<token>& seq) { data->attrs = &seq; }

  void context(shared_ptr<lex_context> ctxt) { data->ctxt = ctxt; }

  void add_skipper(std::string const& s) { data->skippers.insert(s); }

  vector<std::string> get_skippers() const {
    return vector<std::string>(data->skippers.begin(), data->skippers.end());
  }

  void add_init_states(std::string const& s) { data->init_states.insert(s); }

  vector<std::string> get_init_states() const {
    return vector<std::string>(data->init_states.begin(), data->init_states.end());
  }

  void add_state_translation_rule(size_t tok_def_id,
                                  std::string const& on_state,
                                  std::string const& jump_to) {
    auto key_pair = make_pair(tok_def_id, on_state);
    ef_verify(data->state_translations.count(key_pair) == 0);
    data->state_translations.emplace(key_pair, jump_to);
  }

  template <typename IteratorT, typename PassFlagT, typename IdT, typename ContextT>
  void
  operator()(IteratorT& beg, IteratorT& end, PassFlagT& /*flag*/, IdT& id, ContextT& splexer_ctxt) {
    // process token
    std::string str(beg, end);

    // do skip
    std::string splexer_state(splexer_ctxt.get_state_name());
    if (data->skippers.count(splexer_state) == 0) {
      token tok =
          token::make(id, str, data->ctxt->line(), data->ctxt->column(), data->ctxt->file_name());
      data->attrs->push_back(tok);
      data->ctxt->update_position(str);
    }

    // change state
    auto state_translate_key = make_pair(id, splexer_state);
    if (data->state_translations.contains(state_translate_key)) {
      auto next_state = data->state_translations[state_translate_key];
      splexer_ctxt.set_state_name(next_state.c_str());
    }
  }

private:
  shared_ptr<shared_data> data;
};

typedef boost::mpl::vector<std::string> token_types;
typedef splex::lexertl::token<char const*, token_types> splex_token;
typedef splex::lexertl::actor_lexer<splex_token> base_lexer_t;

struct lexer_impl : public splex::lexer<base_lexer_t> {
  unordered_map<std::string, splex::token_def<std::string>> defs;
  unordered_map<size_t, std::string> ids;
  attribute_processor proc;
};

//////////////////////////////////////////////////////////////////////////
// lexer members
lexer::lexer() {
  impl = std::make_shared<lexer_impl>();
}

void lexer::add_patterns(std::initializer_list<definition> patterns) {
  for (auto [name, def] : patterns) {
    impl->self.add_pattern(std::string{name}, std::string{def});
  }
}

void lexer::define_tokens(std::initializer_list<definition> patterns) {
  for (auto [name, def] : patterns) {
    impl->defs[std::string{name}] = std::string{def};
  }
}

void lexer::enable_tokens(char const* state, std::initializer_list<token_description> tokens) {
  for (auto [name, new_state] : tokens) {
    std::string name_s{name};
    auto& def = impl->defs[name_s];
    if (new_state.empty()) {
      impl->self(state).define(def[impl->proc]);
      impl->ids.emplace(def.id(), name_s);
    } else {
      impl->proc.add_state_translation_rule(def.id(), state, std::string{new_state});
    }
  }
}

void lexer::set_skippers(std::initializer_list<std::string_view> skippers) {
  for (auto skipper : skippers) {
    impl->proc.add_skipper(std::string{skipper});
  }
}

void lexer::set_init_states(std::initializer_list<std::string_view> state_list) {
  for (auto state : state_list) {
    impl->proc.add_init_states(std::string{state});
  }
}

std::string const& lexer::get_name(size_t id) {
  return impl->ids[id];
}

size_t lexer::get_id(std::string const& name) {
  return impl->defs[name].id();
}

bool lexer::tokenize(/*INPUTS*/ std::string const& code,
                     shared_ptr<lex_context> ctxt,
                     /*OUTPUT*/ std::vector<token>& seq) {
  impl->proc.output(seq);
  impl->proc.context(ctxt);

  const char* lex_first = &code[0];
  const char* lex_last = &code[0] + code.size();

  // Try to use all lex state for tokenize character sequence.
  std::vector<std::string> tok_states = impl->proc.get_init_states();
  EFLIB_ASSERT_AND_IF(!tok_states.empty(), "Initial state set should not be empty.") {
    return false;
  }

  size_t tok_states_count = tok_states.size();

  int toked_state = 0;  // 0 is no result, 1 is succeeded, 2 is failed.
  int i_state = 0;
  size_t failed_count = 0;
  while (lex_first != lex_last && toked_state == 0) {
    // Use current state, and match as long as possible.
    const char* next_lex_first = lex_first;
    splex::tokenize(next_lex_first, lex_last, *impl, tok_states[i_state].c_str());

    // All was matched, return success(1).
    if (next_lex_first == lex_last) {
      toked_state = 1;
      break;
    }

    // If failed, add failed count.
    if (next_lex_first == lex_first) {
      ++failed_count;
    } else {
      failed_count = 0;
    }

    // If all states was tried and failed, it is really failed.
    if (failed_count == tok_states_count) {
      toked_state = 2;
      break;
    }

    // Otherwise, try next state.
    i_state = (++i_state) % tok_states_count;

    lex_first = next_lex_first;
  }

  bool tokenize_succeed = (toked_state != 2);
  return tokenize_succeed;
}

shared_ptr<lexer_impl> lexer::get_impl() const {
  return impl;
}

bool lexer::begin_incremental() {
  return true;
}

bool lexer::incremental_tokenize(string const& word,
                                 shared_ptr<lex_context> ctxt,
                                 std::vector<token>& seq) {
  return tokenize(word, ctxt, seq);
}

bool lexer::end_incremental(shared_ptr<lex_context> ctxt, std::vector<token>& seq) {
  token tok = token::make(size_t(-1), "", ctxt->line(), ctxt->column(), ctxt->file_name(), true);
  seq.push_back(tok);

  return true;
}

bool lexer::tokenize_with_end(std::string const& code,
                              shared_ptr<lex_context> ctxt,
                              /*OUTPUT*/ std::vector<token>& seq) {
  bool ret = tokenize(code, ctxt, seq);
  if (ret) {
    token tok = token::make(size_t(-1), "", ctxt->line(), ctxt->column(), ctxt->file_name(), true);
    seq.push_back(tok);
  }
  return ret;
}

}  // namespace sasl::parser