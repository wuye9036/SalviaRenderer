#ifndef SASL_SEMANTIC_ABI_INFO_H
#define SASL_SEMANTIC_ABI_INFO_H

#include <sasl/include/semantic/semantic_forward.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_reflection.h>

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
class reflector;

class reflection_impl: public salviar::shader_reflection{
public:
	typedef salviar::sv_layout		sv_layout;
	typedef salviar::semantic_value semantic_value;

	// Friend for reflector could call compute_layout();
	friend class reflector;

	// Implements members of shader_reflection
	virtual salviar::languages		get_language() const;
	virtual eflib::fixed_string		entry_name() const;
	virtual std::vector<sv_layout*>	layouts			(salviar::sv_usage usage) const;
	virtual size_t					layouts_count	(salviar::sv_usage usage) const;
	virtual size_t					total_size		(salviar::sv_usage usage) const;
	
	virtual salviar::sv_layout*		input_sv_layout	(eflib::fixed_string const&) const;
	virtual salviar::sv_layout*		output_sv_layout(semantic_value const&) const;

	virtual bool					has_position_output() const;

	// Impl specific members
	reflection_impl();

	void update_size(size_t sz, salviar::sv_usage usage);

	void module( module_semantic_ptr const& );
	bool is_module( module_semantic_ptr const& ) const;

	void entry(symbol*);
	bool is_entry(symbol*) const;
	
	bool add_input_semantic	(semantic_value const& sem, builtin_types btc, bool is_stream);
	bool add_output_semantic(semantic_value const& sem, builtin_types btc, bool is_stream);
	void add_global_var		(symbol*, sasl::syntax_tree::tynode_ptr btc );

	sv_layout* input_sv_layout(semantic_value const&) const;
	sv_layout* input_sv_layout(symbol*) const;

private:
	sv_layout* alloc_input_layout	(semantic_value const&);
	sv_layout* alloc_input_layout	(symbol*);
	sv_layout* alloc_output_layout	(semantic_value const&);

	module_semantic*	module_sem_;
	symbol*				entry_point_;
	eflib::fixed_string	entry_point_name_;
	sv_layout*			position_output_;

	// Include su_stream_in and su_buffer_in
	typedef boost::unordered_map<semantic_value, sv_layout>			semantic_layout_dict;
	typedef boost::unordered_map<symbol*, sv_layout>				symbol_layout_dict;
	typedef boost::unordered_map<eflib::fixed_string, sv_layout*>	name_layout_dict;

	semantic_layout_dict		semantic_input_layouts_;
	semantic_layout_dict		semantic_output_layouts_;
	symbol_layout_dict			uniform_input_layouts_;

	std::vector<symbol*>		uniform_inputs_;
	std::vector<semantic_value>	semantic_inputs_;
	std::vector<semantic_value> semantic_outputs_;

	name_layout_dict			name_layouts_;

	// The count and offsets of sv_usages
	size_t counts_	[salviar::sv_usage_count];
	size_t offsets_	[salviar::sv_usage_count];
};

END_NS_SASL_SEMANTIC();

#endif