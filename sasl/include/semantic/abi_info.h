#ifndef SASL_SEMANTIC_ABI_INFO_H
#define SASL_SEMANTIC_ABI_INFO_H

#include <sasl/include/semantic/semantic_forward.h>

#include <salviar/include/enums.h>
#include <sasl/enums/builtin_type_code.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SASL_SEMANTIC();

class module_si;
class symbol;

enum storage_types{
	storage_none = 0,
	stream_in,
	stream_out,
	buffer_in,
	buffer_out,
	storage_types_count
};

struct storage_info{
	storage_info();
	int index;
	int offset;
	int size;
	storage_types storage;
	builtin_type_code sv_type;
};

//////////////////////////////////////////////////////////////////////////
// Application binary interface information.
// Used by host and interpolator / rasterizer.
class abi_analyser;

class abi_info{
public:
	// Friend for abi_analyser could call compute_layout();
	friend class abi_analyser;

	abi_info();

	salviar::languages lang;

	void module( boost::shared_ptr<module_si> const& );
	bool is_module( boost::shared_ptr<module_si> const& ) const;

	void entry( boost::shared_ptr<symbol> const& );
	bool is_entry( boost::shared_ptr<symbol> const& ) const;

	bool add_input_semantic( salviar::semantic sem, builtin_type_code btc, bool is_stream );
	bool add_output_semantic( salviar::semantic sem, builtin_type_code btc );
	void add_global_var( boost::shared_ptr<symbol> const&, builtin_type_code btc );

	storage_info* input_storage( salviar::semantic ) const;
	storage_info* input_storage( boost::shared_ptr<symbol> const& ) const;
	storage_info* output_storage( salviar::semantic ) const;
	
	std::vector<storage_info*> storage_infos( storage_types st ) const;

private:
	storage_info* alloc_input_storage( salviar::semantic );
	storage_info* alloc_input_storage( boost::shared_ptr<symbol> const& );
	storage_info* alloc_output_storage( salviar::semantic );

	// Called by abi_analyser after all semantic and global var was set.
	// This function will compute the data layout.
	void compute_layout();

	void compute_input_semantics_layout();
	void compute_output_buffer_layout();
	void compute_output_stream_layout();
	void compute_input_constant_layout();

	module_si* mod;
	symbol* entry_point;

	// Include stream_in and buffer_in
	std::vector< salviar::semantic > sems_in;
	typedef boost::unordered_map< salviar::semantic, storage_info > sem_storages_t;
	sem_storages_t semin_storages;

	// for uniform only.
	std::vector< symbol* > syms_in;
	typedef boost::unordered_map< symbol*, storage_info > sym_storages_t;
	sym_storages_t symin_storages;

	// Include stream_out and buffer_out
	std::vector< salviar::semantic > sems_out;
	boost::unordered_map< salviar::semantic, storage_info > semout_storages;

	// The count and offsets of 
	int counts[sasl::semantic::storage_types_count];
	int offsets[sasl::semantic::storage_types_count];
};

END_NS_SASL_SEMANTIC();

#endif