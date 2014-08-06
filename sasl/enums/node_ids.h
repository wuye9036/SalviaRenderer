#pragma once

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/enum.h>
#include <functional>

enum class node_ids: uint64_t
{
	expression_statement = UINT64_C( 1125899906842631 ),
	member_expression = UINT64_C( 562949953421322 ),
	tynode = UINT64_C( 281479271677952 ),
	unary_expression = UINT64_C( 562949953421315 ),
	for_statement = UINT64_C( 1125899906842633 ),
	initializer = UINT64_C( 2251799813685248 ),
	function_type = UINT64_C( 281479271677956 ),
	variable_declaration = UINT64_C( 281474976710657 ),
	cond_expression = UINT64_C( 562949953421319 ),
	case_label = UINT64_C( 18014398509481986 ),
	compound_statement = UINT64_C( 1125899906842630 ),
	typedef_definition = UINT64_C( 281474976710658 ),
	struct_type = UINT64_C( 281479271677955 ),
	label = UINT64_C( 18014398509481984 ),
	function_full_def = UINT64_C( 281474976710662 ),
	while_statement = UINT64_C( 1125899906842627 ),
	program = UINT64_C( 9007199254740992 ),
	builtin_type = UINT64_C( 281479271677953 ),
	switch_statement = UINT64_C( 1125899906842629 ),
	statement = UINT64_C( 1125899906842624 ),
	expression_initializer = UINT64_C( 2251799813685249 ),
	cast_expression = UINT64_C( 562949953421316 ),
	if_statement = UINT64_C( 1125899906842626 ),
	parameter = UINT64_C( 281474976710660 ),
	constant_expression = UINT64_C( 562949953421313 ),
	node = UINT64_C( 0 ),
	variable_expression = UINT64_C( 562949953421314 ),
	dowhile_statement = UINT64_C( 1125899906842628 ),
	function_def = UINT64_C( 281474976710661 ),
	parameter_full = UINT64_C( 281474976710659 ),
	ident_label = UINT64_C( 18014398509481985 ),
	declaration = UINT64_C( 281474976710656 ),
	array_type = UINT64_C( 281479271677954 ),
	jump_statement = UINT64_C( 1125899906842632 ),
	alias_type = UINT64_C( 281479271677957 ),
	binary_expression = UINT64_C( 562949953421317 ),
	expression_list = UINT64_C( 562949953421318 ),
	member_initializer = UINT64_C( 2251799813685250 ),
	labeled_statement = UINT64_C( 1125899906842634 ),
	declaration_statement = UINT64_C( 1125899906842625 ),
	index_expression = UINT64_C( 562949953421320 ),
	declarator = UINT64_C( 281474976710664 ),
	null_declaration = UINT64_C( 281474976710663 ),
	identifier = UINT64_C( 4503599627370496 ),
	expression = UINT64_C( 562949953421312 ),
	call_expression = UINT64_C( 562949953421321 )
};

void register_enum_name( std::function<void (char const*, node_ids)> const& reg_fn );

