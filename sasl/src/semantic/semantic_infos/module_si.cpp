#include <sasl/include/semantic/semantic_infos.h>

#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_manager.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <salviar/include/enums.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using ::sasl::common::compiler_info_manager;
using ::sasl::syntax_tree::tynode;

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
	typemgr = pety_t::create();
	rootsym = symbol::create_root( boost::shared_ptr<node>() );
	typemgr->root_symbol(rootsym);
}

shared_ptr<class pety_t> module_si::pety() const{
	return typemgr;
}

shared_ptr<symbol> module_si::root() const{
	return rootsym;
}

shared_ptr<compiler_info_manager> module_si::compiler_infos() const{
	return compinfo;
}

vector< shared_ptr<symbol> > const& module_si::globals() const{
	return gvars;
}

vector< shared_ptr<symbol> >& module_si::globals(){
	return gvars;
}

vector< shared_ptr<symbol> > const&  module_si::functions() const{
	return fns;
}

vector< shared_ptr<symbol> >&  module_si::functions(){
	return fns;
}

vector< shared_ptr<symbol> > const& module_si::intrinsics() const{
	return intr;
}

vector< shared_ptr<symbol> >& module_si::intrinsics(){
	return intr;
}

END_NS_SASL_SEMANTIC();