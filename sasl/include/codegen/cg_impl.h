#ifndef SASL_CODEGEN_CG_IMPL_H
#define SASL_CODEGEN_CG_IMPL_H

#include <sasl/include/codegen/forward.h>

#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/codegen/cgs_sisd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/enable_if.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
	namespace semantic{
		class module_semantic;
		class abi_info;
		class caster_t;
		class symbol;
	}
}

namespace llvm{

	class ConstantFolder;
	class LLVMContext;
	class Module;
	class Type;
	class TargetData;
	
	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
        > class IRBuilder;
    typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
        DefaultIRBuilder;
}

struct builtin_types;

BEGIN_NS_SASL_CODEGEN();

class  cg_module;
class  cg_module_impl;
struct node_context;

class cg_impl: public sasl::syntax_tree::syntax_tree_visitor
{
public:
	boost::shared_ptr<cg_module> generated_module() const;
	bool generate(
		boost::shared_ptr<sasl::semantic::module_semantic> const& msem,
		sasl::semantic::abi_info const* abii
		);

	// Get context by node.
	node_context* node_ctxt(sasl::syntax_tree::node const* n, bool create_if_need = false);

protected:
	cg_impl();
	~cg_impl();

	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( constant_expression );
	SASL_VISIT_DCL( variable_expression );
	SASL_VISIT_DCL( binary_expression );
	SASL_VISIT_DCL( call_expression );
	SASL_VISIT_DCL( index_expression );

	SASL_VISIT_DCL( declaration ){ data = data; (&v); }
	
	SASL_VISIT_DCL( builtin_type );
	SASL_VISIT_DCL( parameter_full );
	SASL_VISIT_DCL( function_full_def );
	SASL_VISIT_DCL( struct_type );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( declarator );
	
	SASL_VISIT_DCL( expression_initializer );

	SASL_VISIT_DCL( expression_statement );
	SASL_VISIT_DCL( declaration_statement );
	SASL_VISIT_DCL( jump_statement );

	SASL_VISIT_DCL( program );

	/// It is called in program visitor BEFORE declaration was visited.
	/// If any additional initialization you want to add before visit, override it.
	/// DONT FORGET call parent function before your code.
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program ) = 0;
	SASL_SPECIFIC_VISIT_DCL( process_intrinsics, program );
	
	SASL_SPECIFIC_VISIT_DCL( visit_member_declarator, declarator );
	SASL_SPECIFIC_VISIT_DCL( visit_global_declarator, declarator );
	SASL_SPECIFIC_VISIT_DCL( visit_local_declarator , declarator );

	SASL_SPECIFIC_VISIT_DCL( create_fnsig , function_full_def );
	SASL_SPECIFIC_VISIT_DCL( create_fnargs, function_full_def );
	SASL_SPECIFIC_VISIT_DCL( create_fnbody, function_full_def );

	SASL_SPECIFIC_VISIT_DCL( visit_return	, jump_statement );
	SASL_SPECIFIC_VISIT_DCL( visit_continue	, jump_statement ) = 0;
	SASL_SPECIFIC_VISIT_DCL( visit_break	, jump_statement ) = 0;

	SASL_SPECIFIC_VISIT_DCL( bin_assign	, binary_expression );
	SASL_SPECIFIC_VISIT_DCL( bin_logic	, binary_expression ) = 0;
	
	// Easy to visit child with context data.
	template <typename NodeT> void visit_child(boost::shared_ptr<NodeT> const& child);
	void visit_child(sasl::syntax_tree::node* child);

	template <typename NodeT>
	node_context* node_ctxt( boost::shared_ptr<NodeT> const&, bool create_if_need = false );
	template <typename NodeT>
	node_context* node_ctxt( NodeT const&, bool create_if_need = false,
		typename boost::disable_if< std::is_pointer<NodeT> >::type* = NULL );

	// Direct access member from module.
	llvm::DefaultIRBuilder*	builder() const;
	llvm::LLVMContext&		context() const;
	llvm::Module*			module() const;
	
protected:
	virtual cg_service*		service() const = 0;
	virtual abis::id		local_abi( bool c_compatible ) const = 0;
	sasl::semantic::symbol* find_symbol(std::string const&);
	cg_function*			get_function( std::string const& name ) const;

	// Store global informations
	boost::shared_ptr<sasl::semantic::module_semantic>
										sem_;
	boost::shared_ptr<module_context>	ctxt_;
	boost::shared_ptr<cg_module_impl>	llvm_mod_;
	sasl::semantic::abi_info const*		abii;
	boost::shared_ptr<sasl::semantic::caster_t>
										caster;		///< For type conversation.
	llvm::TargetData const *			target_data;
	cg_service*							service_;

	// Status
	bool				semantic_mode_;
	bool				msc_compatible_;
	insert_point_t		continue_to_;
	insert_point_t		break_to_;
	cg_type*			current_cg_type_;		///< Type information used by declarator.
	node_context*		parent_struct_;
	llvm::BasicBlock*	block_;
	sasl::semantic::symbol*
						current_symbol_;		///< Current symbol scope.
	sasl::syntax_tree::node*
						variable_to_initialize_;///< The variable which will pass in initializer to generate initialization code.
};

END_NS_SASL_CODEGEN()

#endif
