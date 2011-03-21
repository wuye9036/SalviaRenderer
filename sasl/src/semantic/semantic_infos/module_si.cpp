#include <sasl/include/semantic/semantic_infos.h>

#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_manager.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <softart/include/enums.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using ::sasl::common::compiler_info_manager;
using ::sasl::syntax_tree::type_specifier;

using ::boost::addressof;
using ::boost::shared_ptr;
using ::boost::unordered_map;

using ::std::vector;

BEGIN_NS_SASL_SEMANTIC();

////////////////////////////////
// global semantic
module_si::module_si()
{
	compinfo = compiler_info_manager::create();
	typemgr = type_manager::create();
	rootsym = symbol::create_root( boost::shared_ptr<node>() );
	typemgr->root_symbol(rootsym);
}

shared_ptr<class type_manager> module_si::type_manager() const{
	return typemgr;
}

shared_ptr<symbol> module_si::root() const{
	return rootsym;
}

shared_ptr<compiler_info_manager> module_si::compiler_infos() const{
	return compinfo;
}

vector< shared_ptr<symbol> > const& module_si::globals() const{
	return global_syms;
}

void module_si::add_global( shared_ptr<symbol> v ){
	global_syms.push_back(v);
}

std::vector<softart::semantic> const& module_si::used_semantics() const{
	return used_sems;
}

void module_si::mark_semantic( softart::semantic const& s ){
	vector<softart::semantic>::iterator it = lower_bound( used_sems.begin(), used_sems.end(), s );

	if( it == used_sems.end() || *it != s ){
		used_sems.insert( it, s );
	}
}

storage_info const* module_si::storage( softart::semantic sem ) const{
	unordered_map< softart::semantic, storage_info >::const_iterator it = sem_storages.find( sem );
	return it == sem_storages.end() ? NULL : addressof( it->second );
}

storage_info const* module_si::storage( shared_ptr<symbol> const& g_var ) const{
	shared_ptr<storage_si> ssi = extract_semantic_info<storage_si>( g_var->node() );
	return ssi ? addressof( ssi->storage() ) : NULL;
}

void module_si::calculate_storage( softart::languages lang ){
	if( lang == softart::lang_none ){ return; }

	if( lang == softart::lang_vertex_sl ){
		// Did not process memory alignment.
		// It is correspond packed structure in LLVM.

		int sin_idx = 0;
		// int sout_idx = 0;
		int rin_idx = 0;
		int rout_idx = 0;

		int sin_offset = 0;
		// int sout_offset = 0;
		int rin_offset = 0;
		int rout_offset = 0;

		int storage_size = 0;

		// Calculate semantics
		BOOST_FOREACH( softart::semantic sem, used_sems )
		{
			switch( static_cast<softart::semantic>( softart::semantic_base(sem) ) ){
			case softart::SV_Position:
				storage_size = sizeof(float*);
				sem_storages[sem].index = sin_idx++;
				sem_storages[sem].offset = sin_offset;
				sem_storages[sem].size = storage_size;
				sin_offset += storage_size;
				break;

			case softart::SV_Texcoord:
				storage_size = sizeof(float*);
				sem_storages[sem].index = sin_idx++;
				sem_storages[sem].offset = sin_offset;
				sem_storages[sem].size = storage_size;
				sin_offset += storage_size;
				break;

			case softart::SV_RPosition:
				storage_size = sizeof(float) * 4;
				sem_storages[sem].index = rout_idx++;
				sem_storages[sem].offset = rout_offset;
				sem_storages[sem].size = storage_size;
				rout_offset += storage_size;
				break;
			default:
				EFLIB_ASSERT_UNIMPLEMENTED();
			} // switch

		} //BOOST_FOREACH

		// Calculate globals
		BOOST_FOREACH( shared_ptr<symbol> sym, global_syms ){
			shared_ptr<storage_si> ssi = extract_semantic_info<storage_si>( sym->node() );
			shared_ptr<type_specifier> ti = ssi->type_info();
			assert(ti);

			if( ti->is_builtin() ){
				storage_size = static_cast<int>( sasl_ehelper::storage_size( ti->value_typecode ) );

				ssi->storage().index = rin_idx++;
				ssi->storage().offset = rin_offset;
				ssi->storage().size = storage_size;
				rin_offset += storage_size;
			} else {
				EFLIB_ASSERT_UNIMPLEMENTED0( "Don't support un-build-in type as global yet.");
			}
		}
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
}

END_NS_SASL_SEMANTIC();