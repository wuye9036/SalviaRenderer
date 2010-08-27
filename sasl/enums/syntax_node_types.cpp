
#include "./syntax_node_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const syntax_node_types syntax_node_types::expression_statement ( 844424930131975 );
const syntax_node_types syntax_node_types::member_expression ( 562949953421322 );
const syntax_node_types syntax_node_types::unary_expression ( 562949953421315 );
const syntax_node_types syntax_node_types::for_statement ( 844424930131977 );
const syntax_node_types syntax_node_types::initializer ( 1125899906842624 );
const syntax_node_types syntax_node_types::function_type ( 281479271677956 );
const syntax_node_types syntax_node_types::variable_declaration ( 281474976710657 );
const syntax_node_types syntax_node_types::cond_expression ( 562949953421319 );
const syntax_node_types syntax_node_types::type_specifier ( 281479271677952 );
const syntax_node_types syntax_node_types::compound_statement ( 844424930131974 );
const syntax_node_types syntax_node_types::typedef_definition ( 281474976710658 );
const syntax_node_types syntax_node_types::struct_type ( 281479271677955 );
const syntax_node_types syntax_node_types::while_statement ( 844424930131971 );
const syntax_node_types syntax_node_types::program ( 1688849860263936 );
const syntax_node_types syntax_node_types::switch_statement ( 844424930131973 );
const syntax_node_types syntax_node_types::statement ( 844424930131968 );
const syntax_node_types syntax_node_types::cast_expression ( 562949953421316 );
const syntax_node_types syntax_node_types::if_statement ( 844424930131970 );
const syntax_node_types syntax_node_types::parameter ( 281474976710659 );
const syntax_node_types syntax_node_types::constant_expression ( 562949953421313 );
const syntax_node_types syntax_node_types::node ( 0 );
const syntax_node_types syntax_node_types::variable_expression ( 562949953421314 );
const syntax_node_types syntax_node_types::dowhile_statement ( 844424930131972 );
const syntax_node_types syntax_node_types::declaration ( 281474976710656 );
const syntax_node_types syntax_node_types::array_type ( 281479271677954 );
const syntax_node_types syntax_node_types::jump_statement ( 844424930131976 );
const syntax_node_types syntax_node_types::buildin_type ( 281479271677953 );
const syntax_node_types syntax_node_types::binary_expression ( 562949953421317 );
const syntax_node_types syntax_node_types::expression_list ( 562949953421318 );
const syntax_node_types syntax_node_types::member_initializer ( 1125899906842626 );
const syntax_node_types syntax_node_types::declaration_statement ( 844424930131969 );
const syntax_node_types syntax_node_types::index_expression ( 562949953421320 );
const syntax_node_types syntax_node_types::expression_initializer ( 1125899906842625 );
const syntax_node_types syntax_node_types::identifier ( 1407374883553280 );
const syntax_node_types syntax_node_types::expression ( 562949953421312 );
const syntax_node_types syntax_node_types::call_expression ( 562949953421321 );

 
struct enum_hasher: public std::unary_function< syntax_node_types, std::size_t> {
	std::size_t operator()( syntax_node_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_syntax_node_types {
private:
	boost::unordered_map< syntax_node_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, syntax_node_types > name_to_enum;

public:
	dict_wrapper_syntax_node_types(){
		enum_to_name.insert( std::make_pair( syntax_node_types::expression_statement, "expression_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::member_expression, "member_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::unary_expression, "unary_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::for_statement, "for_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::initializer, "initializer" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::function_type, "function_type" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::variable_declaration, "variable_declaration" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::cond_expression, "cond_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::type_specifier, "type_specifier" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::compound_statement, "compound_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::typedef_definition, "typedef_definition" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::struct_type, "struct_type" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::while_statement, "while_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::program, "program" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::switch_statement, "switch_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::statement, "statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::cast_expression, "cast_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::if_statement, "if_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::parameter, "parameter" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::constant_expression, "constant_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::node, "node" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::variable_expression, "variable_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::dowhile_statement, "dowhile_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::declaration, "declaration" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::array_type, "array_type" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::jump_statement, "jump_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::buildin_type, "buildin_type" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::binary_expression, "binary_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::expression_list, "expression_list" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::member_initializer, "member_initializer" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::declaration_statement, "declaration_statement" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::index_expression, "index_expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::expression_initializer, "expression_initializer" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::identifier, "identifier" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::expression, "expression" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::call_expression, "call_expression" ) );

		name_to_enum.insert( std::make_pair( "expression_statement", syntax_node_types::expression_statement ) );
		name_to_enum.insert( std::make_pair( "member_expression", syntax_node_types::member_expression ) );
		name_to_enum.insert( std::make_pair( "unary_expression", syntax_node_types::unary_expression ) );
		name_to_enum.insert( std::make_pair( "for_statement", syntax_node_types::for_statement ) );
		name_to_enum.insert( std::make_pair( "initializer", syntax_node_types::initializer ) );
		name_to_enum.insert( std::make_pair( "function_type", syntax_node_types::function_type ) );
		name_to_enum.insert( std::make_pair( "variable_declaration", syntax_node_types::variable_declaration ) );
		name_to_enum.insert( std::make_pair( "cond_expression", syntax_node_types::cond_expression ) );
		name_to_enum.insert( std::make_pair( "type_specifier", syntax_node_types::type_specifier ) );
		name_to_enum.insert( std::make_pair( "compound_statement", syntax_node_types::compound_statement ) );
		name_to_enum.insert( std::make_pair( "typedef_definition", syntax_node_types::typedef_definition ) );
		name_to_enum.insert( std::make_pair( "struct_type", syntax_node_types::struct_type ) );
		name_to_enum.insert( std::make_pair( "while_statement", syntax_node_types::while_statement ) );
		name_to_enum.insert( std::make_pair( "program", syntax_node_types::program ) );
		name_to_enum.insert( std::make_pair( "switch_statement", syntax_node_types::switch_statement ) );
		name_to_enum.insert( std::make_pair( "statement", syntax_node_types::statement ) );
		name_to_enum.insert( std::make_pair( "cast_expression", syntax_node_types::cast_expression ) );
		name_to_enum.insert( std::make_pair( "if_statement", syntax_node_types::if_statement ) );
		name_to_enum.insert( std::make_pair( "parameter", syntax_node_types::parameter ) );
		name_to_enum.insert( std::make_pair( "constant_expression", syntax_node_types::constant_expression ) );
		name_to_enum.insert( std::make_pair( "node", syntax_node_types::node ) );
		name_to_enum.insert( std::make_pair( "variable_expression", syntax_node_types::variable_expression ) );
		name_to_enum.insert( std::make_pair( "dowhile_statement", syntax_node_types::dowhile_statement ) );
		name_to_enum.insert( std::make_pair( "declaration", syntax_node_types::declaration ) );
		name_to_enum.insert( std::make_pair( "array_type", syntax_node_types::array_type ) );
		name_to_enum.insert( std::make_pair( "jump_statement", syntax_node_types::jump_statement ) );
		name_to_enum.insert( std::make_pair( "buildin_type", syntax_node_types::buildin_type ) );
		name_to_enum.insert( std::make_pair( "binary_expression", syntax_node_types::binary_expression ) );
		name_to_enum.insert( std::make_pair( "expression_list", syntax_node_types::expression_list ) );
		name_to_enum.insert( std::make_pair( "member_initializer", syntax_node_types::member_initializer ) );
		name_to_enum.insert( std::make_pair( "declaration_statement", syntax_node_types::declaration_statement ) );
		name_to_enum.insert( std::make_pair( "index_expression", syntax_node_types::index_expression ) );
		name_to_enum.insert( std::make_pair( "expression_initializer", syntax_node_types::expression_initializer ) );
		name_to_enum.insert( std::make_pair( "identifier", syntax_node_types::identifier ) );
		name_to_enum.insert( std::make_pair( "expression", syntax_node_types::expression ) );
		name_to_enum.insert( std::make_pair( "call_expression", syntax_node_types::call_expression ) );

	}

	std::string to_name( syntax_node_types const& val ){
		boost::unordered_map< syntax_node_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	syntax_node_types from_name( const std::string& name){
		boost::unordered_map< std::string, syntax_node_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_syntax_node_types s_dict;

std::string syntax_node_types::to_name( const syntax_node_types& enum_val){
	return s_dict.to_name(enum_val);
}

syntax_node_types syntax_node_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

