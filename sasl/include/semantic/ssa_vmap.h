#ifndef SASL_SEMANTIC_SSA_VMAP_H
#define SASL_SEMANTIC_SSA_VMAP_H

#include <sasl/include/semantic/semantic_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SASL_SEMANTIC();

struct block_t;
struct function_t;
struct instruction_t;
struct value_t;
struct variable_t;

struct block_vmap
{
	block_t* block;
	typedef std::pair<instruction_t*, value_t*> pos_value_pair_t;
	boost::unordered_map< variable_t*, std::vector<pos_value_pair_t> > block_variables;
};

class function_vmap
{
public:
	void		construct_vmap( function_t* fn );
	void		store( instruction_t* position, variable_t* var, value_t* v );
	value_t*	load ( instruction_t* position, variable_t* var );

private:
	boost::unordered_map<block_t*, block_vmap> block_variables;
};

END_NS_SASL_SEMANTIC();

#endif