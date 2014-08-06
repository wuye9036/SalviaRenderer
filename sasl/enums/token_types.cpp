
#include "./token_types.h"


void register_enum_name( std::function<void (char const*, token_types)> const& reg_fn )
{
	reg_fn("_comment", token_types::_comment);
	reg_fn("_preprocessor", token_types::_preprocessor);
	reg_fn("_operator", token_types::_operator);
	reg_fn("_whitespace", token_types::_whitespace);
	reg_fn("_constant", token_types::_constant);
	reg_fn("_newline", token_types::_newline);
	reg_fn("_identifier", token_types::_identifier);
	reg_fn("_keyword", token_types::_keyword);

}

