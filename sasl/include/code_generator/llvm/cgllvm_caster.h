#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_TYPE_CONVERTER_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_TYPE_CONVERTER_H

#include <sasl/include/code_generator/forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl{
	namespace semantic{
		class caster_t;
		class pety_t;
		class symbol;
	}
	namespace syntax_tree{
		struct node;
	}
}

namespace llvm{
	class IRBuilderBase;
	class Value;
}

BEGIN_NS_SASL_CODE_GENERATOR();

struct	node_context;
class	cg_service;

typedef boost::function<
		node_context* ( boost::shared_ptr<sasl::syntax_tree::node> const& )
	> get_ctxt_fn;

boost::shared_ptr< ::sasl::semantic::caster_t> create_caster(
		get_ctxt_fn const& get_ctxt,
		cg_service* cgs
		);

void add_builtin_casts(
	boost::shared_ptr< ::sasl::semantic::caster_t> caster,
	sasl::semantic::pety_t* typemgr
	);

END_NS_SASL_CODE_GENERATOR();

#endif
