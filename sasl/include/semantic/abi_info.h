#ifndef SASL_SEMANTIC_ABI_INFO_H
#define SASL_SEMANTIC_ABI_INFO_H

#include <sasl/include/semantic/semantic_forward.h>

#include <salviar/include/shader.h>
#include <salviar/include/shader_abi.h>

#include <sasl/enums/builtin_types.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SASL_SEMANTIC();

class module_si;
class symbol;

//////////////////////////////////////////////////////////////////////////
// Application binary interface information.
// Used by host and interpolator / rasterizer.
class abi_analyser;

class abi_info: public salviar::shader_abi{
public:
	typedef salviar::storage_info	storage_info_t;
	typedef salviar::semantic_value	semantic_value_t;

	// Friend for abi_analyser could call compute_layout();
	friend class abi_analyser;

	// Implements members of shader_abi
	
	std::string entry_name() const;

	std::vector<storage_info_t*> storage_infos( salviar::storage_classifications sclass ) const;
	size_t total_size( salviar::storage_classifications sclass ) const;

	storage_info_t* input_storage( std::string const& ) const;
	storage_info_t* output_storage( semantic_value_t const& ) const;

	// End members of shader_abi

	abi_info();

	salviar::languages lang;

	void module( boost::shared_ptr<module_si> const& );
	bool is_module( boost::shared_ptr<module_si> const& ) const;

	void entry( boost::shared_ptr<symbol> const& );
	bool is_entry( boost::shared_ptr<symbol> const& ) const;
	

	bool add_input_semantic( semantic_value_t const& sem, builtin_types btc, bool is_stream );
	bool add_output_semantic( semantic_value_t const& sem, builtin_types btc );
	void add_global_var( boost::shared_ptr<symbol> const&, builtin_types btc );

	storage_info_t* input_storage( semantic_value_t const& ) const;
	storage_info_t* input_storage( boost::shared_ptr<symbol> const& ) const;

private:
	storage_info_t* alloc_input_storage( semantic_value_t const& );
	storage_info_t* alloc_input_storage( boost::shared_ptr<symbol> const& );
	storage_info_t* alloc_output_storage( semantic_value_t const& );

	// Called by abi_analyser after all semantic and global var was set.
	// This function will compute the data layout.
	void compute_layout();

	void compute_input_semantics_layout();
	void compute_output_buffer_layout();
	void compute_output_stream_layout();
	void compute_input_constant_layout();

	module_si* mod;
	symbol* entry_point;
	std::string entry_point_name;

	// Include sc_stream_in and sc_buffer_in
	std::vector< semantic_value_t > sems_in;
	typedef boost::unordered_map< semantic_value_t, storage_info_t > sem_storages_t;
	sem_storages_t semin_storages;

	// for uniform only.
	std::vector< symbol* > syms_in;
	typedef boost::unordered_map< symbol*, storage_info_t > sym_storages_t;
	sym_storages_t symin_storages;
	typedef boost::unordered_map< std::string, storage_info_t* > name_storages_t;
	name_storages_t name_storages;

	// Include sc_stream_out and sc_buffer_out
	std::vector< semantic_value_t > sems_out;
	boost::unordered_map< semantic_value_t, storage_info_t > semout_storages;

	// The count and offsets of 
	int counts[salviar::storage_classifications_count];
	int offsets[salviar::storage_classifications_count];
};

END_NS_SASL_SEMANTIC();

#endif