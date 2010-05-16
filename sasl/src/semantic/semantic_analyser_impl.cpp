#include <sasl/include/semantic/semantic_analyser_impl.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/semantic/semantic_error.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/symbol_infos.h>
#include <sasl/include/semantic/symbol_scope.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <boost/assign/list_of.hpp>

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::semantic::errors::semantic_error;
using ::sasl::common::compiler_info_manager;

semantic_analyser_impl::semantic_analyser_impl( boost::shared_ptr<compiler_info_manager> infomgr )
	: infomgr( infomgr ){}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::unary_expression& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::cast_expression& v){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::binary_expression& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::expression_list& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::cond_expression& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::index_expression& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::call_expression& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::member_expression& v ){}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::constant_expression& v ){
	using ::sasl::syntax_tree::constant_expression;

	// add value symbol info to current node.
	boost::shared_ptr<const_value_symbol_info> vsyminfo = get_or_create_symbol_info<const_value_symbol_info>(cursym->node());
	vsyminfo->constant_value_literal( v.value_tok->lit, v.ctype );
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::identifier& v ){}

// declaration & type specifier
void semantic_analyser_impl::visit( ::sasl::syntax_tree::initializer& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::expression_initializer& v ){
	v.init_expr->accept(this);
}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::member_initializer& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::declaration& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::variable_declaration& v ){
	using ::boost::assign::list_of;

	symbol_scope sc( v.name->lit, v.handle(), cursym );

	// process variable type
	boost::shared_ptr<type_specifier> vartype = v.type_info;
	vartype->accept( this );
	boost::shared_ptr<type_symbol_info> typesyminfo = extract_symbol_info<type_symbol_info>(v);
	
	// check type.
	if ( typesyminfo->type_type() == type_types::buildin ){
		// TODO: ALLOCATE BUILD-IN TYPED VAR.
	} else if ( typesyminfo->type_type() == type_types::composited ){
		// TODO: ALLOCATE COMPOSITED TYPED VAR.
	} else if ( typesyminfo->type_type() == type_types::alias ){
		if ( typesyminfo->full_type() ){
			// TODO: ALLOCATE ACTUAL
		} else {
			infomgr->add_info( semantic_error::create( compiler_informations::uses_a_undef_type,
				v.handle(), list_of( typesyminfo->full_type() ) )
				);
			// remove created symbol
			cursym->remove_from_tree();
			return;
		}
	}

	// process initializer
	v.init->accept( this );
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::type_definition& v ){
	using ::sasl::syntax_tree::type_definition;
	using ::boost::assign::list_of;
	const std::string& alias_str = v.ident->lit;
	boost::shared_ptr<symbol> existed_sym = cursym->find_all( alias_str );
	if ( existed_sym ){
		// if the symbol is used and is not a type node, it must be redifinition.
		// else compare the type.
		if ( !existed_sym->node()->node_class().included( syntax_node_types::type_specifier ) ){
			infomgr->add_info( 
				semantic_error::create( compiler_informations::redef_cannot_overloaded,
				v.handle(),	list_of(existed_sym->node()) )
					);
			return;
		}
	}

	// process type node.
	// remove old sym from symbol table.
	cursym->remove_child( v.ident->lit );
	{
		symbol_scope sc( v.ident->lit, v.handle(), cursym );

		v.type_info->accept(this);
		boost::shared_ptr<type_symbol_info> new_tsi = extract_symbol_info<type_symbol_info>(v);

		// if this symbol is usable, process type node.
		if ( existed_sym ){
			boost::shared_ptr<type_symbol_info> existed_tsi = existed_sym->symbol_info<type_symbol_info>();
			if ( !is_equal(existed_tsi->full_type(), new_tsi->full_type()) ){
				// if new symbol is different from the old, semantic error.
				// The final effect is that the new definition overwrites the old one.

				infomgr->add_info( 
					semantic_error::create( compiler_informations::redef_diff_basic_type,
					v.handle(),	list_of(existed_sym->node()) )
					);
			} 
		}
		// else if the same. do not updated.
		// NOTE:
		//   MAYBE IT NEEDS COMBINE OLD AND NEW SYMBOL INFOS UNDER SOME CONDITIONS. 
		//   BUT I CAN NOT FIND OUT ANY EXAMPLE.
	}
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::type_specifier& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::buildin_type& v ){
	using ::sasl::semantic::get_or_create_symbol_info;

	// create type information on current symbol.
	// for e.g. create type info onto a variable node.
	boost::shared_ptr<type_symbol_info> tsyminfo = get_or_create_symbol_info<type_symbol_info>( cursym->node() );
	tsyminfo->type_type( type_types::buildin );
	tsyminfo->full_type( boost::shared_polymorphic_cast<type_specifier>(v.handle()) );
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::type_identifier& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::qualified_type& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::array_type& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::struct_type& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::parameter& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::function_type& v ){}

// statement
void semantic_analyser_impl::visit( ::sasl::syntax_tree::statement& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::declaration_statement& v ){
	v.decl->accept(this);
}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::if_statement& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::while_statement& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::dowhile_statement& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::case_label& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::switch_statement& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::compound_statement& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::expression_statement& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::jump_statement& v ){}

// program
void semantic_analyser_impl::visit( ::sasl::syntax_tree::program& v ){
	is_local = false;
	cursym = symbol::create_root( v.handle() );

	for( size_t i = 0; i < v.decls.size(); ++i){
		v.decls[i]->accept(this);
	}
}

END_NS_SASL_SEMANTIC();