#ifndef SASL_CODEGEN_CG_CASTER_H
#define SASL_CODEGEN_CG_CASTER_H

#include <sasl/include/codegen/forward.h>

#include <sasl/include/semantic/caster.h>

#include <memory>
#include <functional>

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

BEGIN_NS_SASL_CODEGEN();

struct	node_context;
class	cg_service;

typedef std::function<
	node_context* (sasl::syntax_tree::node const*)> get_context_fn;

std::shared_ptr< ::sasl::semantic::caster_t> create_cg_caster(
		get_context_fn const&					get_context,
		sasl::semantic::get_semantic_fn const&	get_semantic,
		sasl::semantic::get_tynode_fn const&	get_tynode,
		cg_service* cgs
		);

void add_builtin_casts(
	std::shared_ptr< ::sasl::semantic::caster_t> caster,
	sasl::semantic::pety_t* typemgr
	);

END_NS_SASL_CODEGEN();

#endif
