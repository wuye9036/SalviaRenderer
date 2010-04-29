#include <sasl/include/code_generator/llvm/llvm_generator.h>
#include <sasl/include/syntax_tree/declaration.h>

BEGIN_NS_SASL_CODE_GENERATOR()

using namespace syntax_tree;
using namespace llvm;

llvm_code_generator::llvm_code_generator()
	: ctxt()
{
}

void llvm_code_generator::visit( unary_expression& v ){
	
}

void llvm_code_generator::visit( cast_expression& v){}
void llvm_code_generator::visit( binary_expression& v ){}
void llvm_code_generator::visit( expression_list& v ){}
void llvm_code_generator::visit( cond_expression& v ){}
void llvm_code_generator::visit( index_expression& v ){}
void llvm_code_generator::visit( call_expression& v ){}
void llvm_code_generator::visit( member_expression& v ){}

void llvm_code_generator::visit( constant_expression& v ){}
void llvm_code_generator::visit( constant& v ){}
void llvm_code_generator::visit( identifier& v ){}

// declaration & type specifier
void llvm_code_generator::visit( initializer& v ){}
void llvm_code_generator::visit( expression_initializer& v ){}
void llvm_code_generator::visit( member_initializer& v ){}
void llvm_code_generator::visit( declaration& v ){}
void llvm_code_generator::visit( variable_declaration& v ){}
void llvm_code_generator::visit( type_definition& v ){}
void llvm_code_generator::visit( type_specifier& v ){}
void llvm_code_generator::visit( buildin_type& v ){}
void llvm_code_generator::visit( type_identifier& v ){}
void llvm_code_generator::visit( qualified_type& v ){}
void llvm_code_generator::visit( array_type& v ){}
void llvm_code_generator::visit( struct_type& v ){}
void llvm_code_generator::visit( parameter& v ){}

void llvm_code_generator::visit( function_type& v ){
}

// statement
void llvm_code_generator::visit( statement& v ){}
void llvm_code_generator::visit( declaration_statement& v ){}
void llvm_code_generator::visit( if_statement& v ){}
void llvm_code_generator::visit( while_statement& v ){}
void llvm_code_generator::visit( dowhile_statement& v ){}
void llvm_code_generator::visit( case_label& v ){}
void llvm_code_generator::visit( switch_statement& v ){}
void llvm_code_generator::visit( compound_statement& v ){}
void llvm_code_generator::visit( expression_statement& v ){}
void llvm_code_generator::visit( jump_statement& v ){}

END_NS_SASL_CODE_GENERATOR()