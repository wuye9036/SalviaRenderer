#ifndef SASL_SEMANTIC_ABI_INFO_H
#define SASL_SEMANTIC_ABI_INFO_H

#include <sasl/include/semantic/semantic_forward.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_abi.h>

#include <sasl/enums/builtin_types.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl
{
	namespace syntax_tree
	{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(tynode);
	}
}

BEGIN_NS_SASL_SEMANTIC();

EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
EFLIB_DECLARE_CLASS_SHARED_PTR(symbol);

//////////////////////////////////////////////////////////////////////////
// Application binary interface information.
// Used by host and interpolator / rasterizer.
class abi_analyser;

class abi_info: public salviar::shader_abi{
public:
	typedef salviar::sv_layout		sv_layout_t;
	typedef salviar::semantic_value	semantic_value_t;

	// Friend for abi_analyser could call compute_layout();
	friend class abi_analyser;

	// Implements members of shader_abi
	std::string entry_name() const;

	std::vector<sv_layout_t*> layouts( salviar::sv_usage usage ) const;
	size_t total_size( salviar::sv_usage usage ) const;
	void update_size( size_t sz, salviar::sv_usage usage );

	sv_layout_t* input_sv_layout( std::string const& ) const;
	sv_layout_t* output_sv_layout( semantic_value_t const& ) const;

	// End members of shader_abi

	abi_info();

	salviar::languages lang;

	void module( module_semantic_ptr const& );
	bool is_module( module_semantic_ptr const& ) const;

	void entry(symbol*);
	bool is_entry(symbol*) const;
	
	bool add_input_semantic( semantic_value_t const& sem, builtin_types btc, bool is_stream );
	bool add_output_semantic( semantic_value_t const& sem, builtin_types btc, bool is_stream );
	void add_global_var( symbol*, sasl::syntax_tree::tynode_ptr btc );

	sv_layout_t* input_sv_layout( semantic_value_t const& ) const;
	sv_layout_t* input_sv_layout( symbol* ) const;

private:
	sv_layout_t* alloc_input_storage( semantic_value_t const& );
	sv_layout_t* alloc_input_storage( symbol* );
	sv_layout_t* alloc_output_storage( semantic_value_t const& );

	module_semantic*	module_sem_;
	symbol*				entry_point_;
	std::string			entry_point_name_;

	// Include su_stream_in and su_buffer_in
	std::vector< semantic_value_t > sems_in;
	typedef boost::unordered_map< semantic_value_t, sv_layout_t > sem_storages_t;
	sem_storages_t semin_storages;

	// for uniform only.
	std::vector< symbol* > syms_in;
	typedef boost::unordered_map< symbol*, sv_layout_t > sym_storages_t;
	sym_storages_t symin_storages;
	typedef boost::unordered_map< std::string, sv_layout_t* > name_storages_t;
	name_storages_t name_storages;

	// Include su_stream_out and su_buffer_out
	std::vector< semantic_value_t > sems_out;
	boost::unordered_map< semantic_value_t, sv_layout_t > semout_storages;

	// The count and offsets of 
	int counts[salviar::sv_usage_count];
	int offsets[salviar::sv_usage_count];
};

END_NS_SASL_SEMANTIC();

#endif