#include <sasl/include/semantic/semantic_analyser_impl.h>
#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/semantic/semantic_error.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol_scope.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <boost/assign/list_of.hpp>
#include <boost/scoped_ptr.hpp>

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
	boost::shared_ptr<const_value_semantic_info> vseminfo = get_or_create_semantic_info<const_value_semantic_info>(cursym->node());
	vseminfo->constant_value_literal( v.value_tok->lit, v.ctype );
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
	boost::shared_ptr<type_semantic_info> typeseminfo = extract_semantic_info<type_semantic_info>(v);
	
	// check type.
	if ( typeseminfo->type_type() == type_types::buildin ){
		// TODO: ALLOCATE BUILD-IN TYPED VAR.
	} else if ( typeseminfo->type_type() == type_types::composited ){
		// TODO: ALLOCATE COMPOSITED TYPED VAR.
	} else if ( typeseminfo->type_type() == type_types::alias ){
		if ( typeseminfo->full_type() ){
			// TODO: ALLOCATE ACTUAL
		} else {
			infomgr->add_info( semantic_error::create( compiler_informations::uses_a_undef_type,
				v.handle(), list_of( typeseminfo->full_type() ) )
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
	boost::shared_ptr<symbol> existed_sym = cursym->find_mangled_this( alias_str );
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
		boost::shared_ptr<type_semantic_info> new_tsi = extract_semantic_info<type_semantic_info>(v);

		// if this symbol is usable, process type node.
		if ( existed_sym ){
			boost::shared_ptr<type_semantic_info> existed_tsi = extract_semantic_info<type_semantic_info>( existed_sym->node() );
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
	using ::sasl::semantic::get_or_create_semantic_info;

	// create type information on current symbol.
	// for e.g. create type info onto a variable node.
	boost::shared_ptr<type_semantic_info> tseminfo = get_or_create_semantic_info<type_semantic_info>( cursym->node() );
	tseminfo->type_type( type_types::buildin );
	tseminfo->full_type( boost::shared_polymorphic_cast<type_specifier>(v.handle()) );
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::type_identifier& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::qualified_type& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::array_type& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::struct_type& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::parameter& v ){}
void semantic_analyser_impl::visit( ::sasl::syntax_tree::function_type& v ){
	using ::sasl::semantic::symbol;
	using ::sasl::syntax_tree::function_type;
	using ::sasl::semantic::get_or_create_semantic_info;
	using ::sasl::semantic::extract_semantic_info;

	// if it is only declaration.
	std::string symbol_name;
	std::string unmangled_name = v.name->lit;

	// process parameter types for name mangling.
	v.retval_type->accept( this );
	for( size_t i_param = 0; i_param < v.params.size(); ++i_param ){
		v.params[i_param]->param_type->accept(this);
	}

	std::string mangled_name = mangle_function_name( v.typed_handle<function_type>() );

	bool use_existed_node(false);

	boost::shared_ptr<symbol> existed_sym = cursym->find_mangled_this( unmangled_name );
	if ( existed_sym ) {
		boost::shared_ptr<function_type> existed_node = existed_sym->node()->typed_handle<function_type>();
		if ( !existed_node ){
			// symbol was used, and it is not a function. error.
			// TODO: SEMANTIC ERROR: TYPE REDEFINITION.
		} else {
			// symbol was used, and the older is a function.
			existed_node = cursym->find_mangled_this(mangled_name)->node()->typed_handle<function_type>();
			if ( existed_node ){
				if ( !is_equal( existed_node, v.typed_handle<function_type>() ) ){
					// TODO: BUG ON OVERLOAD SUPPORTING
					// TODO: SEMANTIC ERROR ON OVERLOAD UNSUPPORTED.
				}
				if ( v.is_declaration ){
					// it was had a definition/declaration, and now is a declaration only
					use_existed_node = true;
				} else {
					if ( existed_node->is_declaration ){
						// the older is a declaration, and whatever now is, that's OK.
						use_existed_node = true;
					} else {
						// older function definition v.s. new function definition, conflict...
						// TODO:  SEMANTIC ERROR REDEFINE A FUNCTION.
					}
				}
			} else {
				use_existed_node = false;
			}
		}
	} else {
		use_existed_node = false;
	}

	boost::scoped_ptr<symbol_scope> sc(
		use_existed_node ? new symbol_scope(mangled_name, cursym) : new symbol_scope( mangled_name, unmangled_name, v.handle(), cursym )
		);

	v.symbol( cursym );
	if ( !use_existed_node ){
		// replace old node via new node.
		cursym->relink( v.handle() );
		
		// definition
		if ( !v.is_declaration ){
			// process parameters
			for( size_t i_param = 0; i_param < v.params.size(); ++i_param ){
				v.params[i_param]->accept( this );
			}

			// process statements
			is_local = true;
			for( size_t i_stmt = 0; i_stmt < v.stmts.size(); ++i_stmt ){
				v.stmts[i_stmt]->accept( this );
			}
		}
	}
}

// statement
void semantic_analyser_impl::visit( ::sasl::syntax_tree::statement& v ){
	assert( !"can not reach this point!" );
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::declaration_statement& v ){
	v.decl->accept(this);
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::if_statement& v ){
	v.cond->accept( this );
	v.yes_stmt->accept( this );
	v.no_stmt->accept(this);
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::while_statement& v ){
	v.cond->accept( this );
	v.body->accept( this );
}

void semantic_analyser_impl::visit( ::sasl::syntax_tree::dowhile_statement& v ){
}

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