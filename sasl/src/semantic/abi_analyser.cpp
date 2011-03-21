#include <sasl/include/semantic/abi_analyser.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/addressof.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using boost::addressof;
using boost::shared_ptr;

using std::lower_bound;
using std::vector;

BEGIN_NS_SASL_SEMANTIC();

storage_info::storage_info()
	: index(-1), offset(0), size(0), storage(storage_none)
{}

abi_info::abi_info() : mod(NULL), entry_point(NULL)
{}

void abi_info::module( shared_ptr<module_si> const& v ){
	mod = v.get();
}

bool abi_info::is_module( boost::shared_ptr<module_si> const& v ){
	return mod == v.get();
}

void abi_info::entry( boost::shared_ptr<symbol> const& v ){
	entry_point = v.get();
}

bool abi_info::is_entry( boost::shared_ptr<symbol> const& v ){
	return entry_point == v.get();
}

bool abi_info::add_input_semantic( softart::semantic sem ){
	vector<softart::semantic>::iterator it = std::lower_bound( sems_in.begin(), sems_in.end(), sem );
	if( it != sems_in.end() ){
		if( *it == sem ){
			return false;
		}
	}

	sems_in.insert( it, sem );
	return true;
}

bool abi_info::add_output_semantic( softart::semantic sem ){
	vector<softart::semantic>::iterator it = std::lower_bound( sems_in.begin(), sems_in.end(), sem );
	if( it != sems_in.end() ){
		if( *it == sem ){
			return false;
		}
	}

	sems_in.insert( it, sem );
	return true;
}

void abi_info::add_global_var( shared_ptr<symbol> const& v ){
	syms_in.push_back( v.get() );
}

storage_info* abi_info::input_storage( softart::semantic sem ){
	sem_storages_t::iterator it = semin_storages.find( sem );
	if ( it == semin_storages.end() ){
		return NULL;
	}
	return addressof( it->second );
}

storage_info* abi_info::alloc_input_storage( softart::semantic sem ){
	return addressof( semin_storages[sem] );
}

storage_info* abi_info::input_storage( boost::shared_ptr<symbol> const& v ){
	sym_storages_t::iterator it = symin_storages.find( v.get() );
	if ( it == symin_storages.end() ){
		return NULL;
	}
	return addressof( it->second );
}

storage_info* abi_info::alloc_input_storage( boost::shared_ptr<symbol> const& v ){
	return addressof( symin_storages[v.get()] );
}

storage_info* abi_info::output_storage( softart::semantic sem ){
	sem_storages_t::iterator it = semout_storages.find( sem );
	if ( it == semout_storages.end() ){
		return NULL;
	}
	return addressof( it->second );
}

storage_info* abi_info::alloc_output_storage( softart::semantic sem ){
	return addressof( semout_storages[sem] );
}

//void module_si::calculate_storage( softart::languages lang ){
//	if( lang == softart::lang_none ){ return; }
//
//	if( lang == softart::lang_vertex_sl ){
//		// Did not process memory alignment.
//		// It is correspond packed structure in LLVM.
//
//		int sin_idx = 0;
//		// int sout_idx = 0;
//		int rin_idx = 0;
//		int rout_idx = 0;
//
//		int sin_offset = 0;
//		// int sout_offset = 0;
//		int rin_offset = 0;
//		int rout_offset = 0;
//
//		int storage_size = 0;
//
//		// Calculate semantics
//		BOOST_FOREACH( softart::semantic sem, used_sems )
//		{
//			switch( static_cast<softart::semantic>( softart::semantic_base(sem) ) ){
//			case softart::SV_Position:
//				storage_size = sizeof(float*);
//				sem_storages[sem].index = sin_idx++;
//				sem_storages[sem].offset = sin_offset;
//				sem_storages[sem].size = storage_size;
//				sin_offset += storage_size;
//				break;
//
//			case softart::SV_Texcoord:
//				storage_size = sizeof(float*);
//				sem_storages[sem].index = sin_idx++;
//				sem_storages[sem].offset = sin_offset;
//				sem_storages[sem].size = storage_size;
//				sin_offset += storage_size;
//				break;
//
//			case softart::SV_RPosition:
//				storage_size = sizeof(float) * 4;
//				sem_storages[sem].index = rout_idx++;
//				sem_storages[sem].offset = rout_offset;
//				sem_storages[sem].size = storage_size;
//				rout_offset += storage_size;
//				break;
//			default:
//				EFLIB_ASSERT_UNIMPLEMENTED();
//			} // switch
//
//		} //BOOST_FOREACH
//
//		// Calculate globals
//		BOOST_FOREACH( shared_ptr<symbol> sym, global_syms ){
//			shared_ptr<storage_si> ssi = extract_semantic_info<storage_si>( sym->node() );
//			shared_ptr<type_specifier> ti = ssi->type_info();
//			assert(ti);
//
//			if( ti->is_builtin() ){
//				storage_size = static_cast<int>( sasl_ehelper::storage_size( ti->value_typecode ) );
//
//				ssi->storage().index = rin_idx++;
//				ssi->storage().offset = rin_offset;
//				ssi->storage().size = storage_size;
//				rin_offset += storage_size;
//			} else {
//				EFLIB_ASSERT_UNIMPLEMENTED0( "Don't support un-build-in type as global yet.");
//			}
//		}
//	} else {
//		EFLIB_ASSERT_UNIMPLEMENTED();
//	}
//}
END_NS_SASL_SEMANTIC();