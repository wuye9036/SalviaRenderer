#pragma once

#include <sasl/common/lex_context.h>

#include <sasl/common/token.h>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sasl::parser {

struct lexer_impl;

class lexer {
private:
  using token = sasl::common::token;

public:
  lexer();

  struct definition {
    std::string_view name;
    std::string_view def;
  };

  struct token_description {
    std::string_view name;
    std::string_view new_state = "";
  };

  void add_patterns(std::initializer_list<definition> patterns);
  void define_tokens(std::initializer_list<definition> patterns);
  void enable_tokens(char const* state, std::initializer_list<token_description> tokens);
  void set_skippers(std::initializer_list<std::string_view> skippers);
  void set_init_states(std::initializer_list<std::string_view> state_list);

  std::string const& get_name(size_t id);
  size_t get_id(std::string const& name);

  std::shared_ptr<lexer_impl> get_impl() const;

  bool tokenize_with_end(
      /*INPUTS*/ std::string const& code,
      std::shared_ptr<sasl::common::lex_context> ctxt,
      /*OUTPUT*/ std::vector<token>& seq);

  bool begin_incremental();
  bool incremental_tokenize(std::string const& word,
                            std::shared_ptr<sasl::common::lex_context> ctxt,
                            std::vector<token>& seq);
  bool end_incremental(std::shared_ptr<sasl::common::lex_context> ctxt, std::vector<token>& seq);

private:
  bool tokenize(
      /*INPUTS*/ std::string const& code,
      std::shared_ptr<sasl::common::lex_context> ctxt,
      /*OUTPUT*/ std::vector<token>& seq);
  std::shared_ptr<lexer_impl> impl;
};

}  // namespace sasl::parser
