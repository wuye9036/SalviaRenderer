#ifndef SASL_SEMANTIC_ABI_ANALYSER_H
#define SASL_SEMANTIC_ABI_ANALYSER_H

#include <sasl/include/semantic/semantic_forward.h>

#include <softart/include/enums.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

BEGIN_NS_SASL_SEMANTIC();

class symbol;
class module_si;

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
};

//////////////////////////////////////////////////////////////////////////
// Application binary interface information.
// Used by host and interpolator / rasterizer.

class abi_info{
public:
	abi_info();

	softart::languages lang;

	void module( boost::shared_ptr<module_si> const& );
	bool is_module( boost::shared_ptr<module_si> const& );

	void entry( boost::shared_ptr<symbol> const& );
	bool is_entry( boost::shared_ptr<symbol> const& );

	bool add_input_semantic( softart::semantic sem );
	bool add_output_semantic( softart::semantic sem );
	void add_global_var( boost::shared_ptr<symbol> const& );

	storage_info* storage( softart::semantic );
	storage_info* alloc_storage( softart::semantic );

	storage_info* storage( boost::shared_ptr<symbol> const& );
	storage_info* alloc_storage( boost::shared_ptr<symbol> const& );

private:
	void update_abii();

	module_si* mod;
	symbol* entry_point;

	std::vector< softart::semantic > sems_in;
	boost::unordered_map< softart::semantic, storage_info > semin_storages;
	std::vector< symbol* > syms_in;
	boost::unordered_map< symbol* ,storage_info > syms_in_storages;

	std::vector< softart::semantic > sems_out;
	boost::unordered_map< softart::semantic, storage_info > semout_storages;
};

// If entry of VS and PS was set, match the ABIs to generate interpolating code.
class abi_analyser{
public:
	void entry( boost::shared_ptr<module_si>& mod, std::string const& name, softart::languages lang );
	boost::shared_ptr<symbol> const& entry( softart::languages lang ) const;

	void reset();
	bool update_abi();

	abi_info const* abii( softart::languages lang );

private:

	bool update_vs();
	bool update_ps();
	bool update_bs();

	boost::shared_ptr<module_si> vs_mod;
	boost::shared_ptr<symbol> vs_entry;
	boost::shared_ptr<abi_info> vs_abii;

	boost::shared_ptr<module_si> ps_mod;
	boost::shared_ptr<symbol> ps_entry;
	boost::shared_ptr<abi_info> ps_abii;

	boost::shared_ptr<module_si> bs_mod;
	boost::shared_ptr<symbol> bs_entry;
	boost::shared_ptr<abi_info> bs_abii;
};

END_NS_SASL_SEMANTIC();

#endif