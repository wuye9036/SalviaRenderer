
#include "./syntax_node_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< syntax_node_types, std::size_t> {
	std::size_t operator()( syntax_node_types const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_syntax_node_types {
private:
	boost::unordered_map< syntax_node_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, syntax_node_types > name_to_enum;

	dict_wrapper_syntax_node_types(){}
	
public:
	static dict_wrapper_syntax_node_types& instance();
	
	void insert( syntax_node_types const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
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

dict_wrapper_syntax_node_types& dict_wrapper_syntax_node_types::instance(){
	static dict_wrapper_syntax_node_types inst;
	return inst;
}

std::string syntax_node_types::to_name( const syntax_node_types& enum_val){
	return dict_wrapper_syntax_node_types::instance().to_name(enum_val);
}

syntax_node_types syntax_node_types::from_name( const std::string& name){
	return dict_wrapper_syntax_node_types::instance().from_name(name);
}

std::string syntax_node_types::name() const{
	return to_name( * this );
}



		
syntax_node_types::syntax_node_types( const storage_type& val, const std::string& name ): syntax_node_types::base_type(val){
	syntax_node_types tmp(val);
	dict_wrapper_syntax_node_types::instance().insert( tmp, name );
}

const syntax_node_types syntax_node_types::expression_statement ( UINT64_C( 844424930131975 ), "expression_statement" );
const syntax_node_types syntax_node_types::member_expression ( UINT64_C( 562949953421322 ), "member_expression" );
const syntax_node_types syntax_node_types::unary_expression ( UINT64_C( 562949953421315 ), "unary_expression" );
const syntax_node_types syntax_node_types::for_statement ( UINT64_C( 844424930131977 ), "for_statement" );
const syntax_node_types syntax_node_types::initializer ( UINT64_C( 1125899906842624 ), "initializer" );
const syntax_node_types syntax_node_types::function_type ( UINT64_C( 281479271677956 ), "function_type" );
const syntax_node_types syntax_node_types::variable_declaration ( UINT64_C( 281474976710657 ), "variable_declaration" );
const syntax_node_types syntax_node_types::cond_expression ( UINT64_C( 562949953421319 ), "cond_expression" );
const syntax_node_types syntax_node_types::case_label ( UINT64_C( 1970324836974594 ), "case_label" );
const syntax_node_types syntax_node_types::type_specifier ( UINT64_C( 281479271677952 ), "type_specifier" );
const syntax_node_types syntax_node_types::compound_statement ( UINT64_C( 844424930131974 ), "compound_statement" );
const syntax_node_types syntax_node_types::typedef_definition ( UINT64_C( 281474976710658 ), "typedef_definition" );
const syntax_node_types syntax_node_types::struct_type ( UINT64_C( 281479271677955 ), "struct_type" );
const syntax_node_types syntax_node_types::label ( UINT64_C( 1970324836974592 ), "label" );
const syntax_node_types syntax_node_types::while_statement ( UINT64_C( 844424930131971 ), "while_statement" );
const syntax_node_types syntax_node_types::program ( UINT64_C( 1688849860263936 ), "program" );
const syntax_node_types syntax_node_types::switch_statement ( UINT64_C( 844424930131973 ), "switch_statement" );
const syntax_node_types syntax_node_types::statement ( UINT64_C( 844424930131968 ), "statement" );
const syntax_node_types syntax_node_types::cast_expression ( UINT64_C( 562949953421316 ), "cast_expression" );
const syntax_node_types syntax_node_types::if_statement ( UINT64_C( 844424930131970 ), "if_statement" );
const syntax_node_types syntax_node_types::parameter ( UINT64_C( 281474976710659 ), "parameter" );
const syntax_node_types syntax_node_types::constant_expression ( UINT64_C( 562949953421313 ), "constant_expression" );
const syntax_node_types syntax_node_types::node ( UINT64_C( 0 ), "node" );
const syntax_node_types syntax_node_types::variable_expression ( UINT64_C( 562949953421314 ), "variable_expression" );
const syntax_node_types syntax_node_types::dowhile_statement ( UINT64_C( 844424930131972 ), "dowhile_statement" );
const syntax_node_types syntax_node_types::ident_label ( UINT64_C( 1970324836974593 ), "ident_label" );
const syntax_node_types syntax_node_types::declaration ( UINT64_C( 281474976710656 ), "declaration" );
const syntax_node_types syntax_node_types::array_type ( UINT64_C( 281479271677954 ), "array_type" );
const syntax_node_types syntax_node_types::jump_statement ( UINT64_C( 844424930131976 ), "jump_statement" );
const syntax_node_types syntax_node_types::alias_type ( UINT64_C( 281479271677957 ), "alias_type" );
const syntax_node_types syntax_node_types::buildin_type ( UINT64_C( 281479271677953 ), "buildin_type" );
const syntax_node_types syntax_node_types::binary_expression ( UINT64_C( 562949953421317 ), "binary_expression" );
const syntax_node_types syntax_node_types::expression_list ( UINT64_C( 562949953421318 ), "expression_list" );
const syntax_node_types syntax_node_types::member_initializer ( UINT64_C( 1125899906842626 ), "member_initializer" );
const syntax_node_types syntax_node_types::declaration_statement ( UINT64_C( 844424930131969 ), "declaration_statement" );
const syntax_node_types syntax_node_types::index_expression ( UINT64_C( 562949953421320 ), "index_expression" );
const syntax_node_types syntax_node_types::expression_initializer ( UINT64_C( 1125899906842625 ), "expression_initializer" );
const syntax_node_types syntax_node_types::null_declaration ( UINT64_C( 281474976710660 ), "null_declaration" );
const syntax_node_types syntax_node_types::identifier ( UINT64_C( 1407374883553280 ), "identifier" );
const syntax_node_types syntax_node_types::expression ( UINT64_C( 562949953421312 ), "expression" );
const syntax_node_types syntax_node_types::call_expression ( UINT64_C( 562949953421321 ), "call_expression" );


