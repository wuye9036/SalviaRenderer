#ifndef SASL_SYNTAX_TREE_SYMBOL_INFO_H
#define SASL_SYNTAX_TREE_SYMBOL_INFO_H

class symbol_info{
	// for structure
	size_t size;
	
	// for structure member
	size_t offset;

	// for variables
	size_t binded_register;

	// for code generation
	bool is_intermediate;
	size_t stack_offset;
	size_t aligned_size;	
};

#endif