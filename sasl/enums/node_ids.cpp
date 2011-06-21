
#include "./node_ids.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< node_ids, std::size_t> {
	std::size_t operator()( node_ids const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_node_ids {
private:
	boost::unordered_map< node_ids, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, node_ids > name_to_enum;

	dict_wrapper_node_ids(){}
	
public:
	static dict_wrapper_node_ids& instance();
	
	void insert( node_ids const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( node_ids const& val ){
		boost::unordered_map< node_ids, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	node_ids from_name( const std::string& name){
		boost::unordered_map< std::string, node_ids >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_node_ids& dict_wrapper_node_ids::instance(){
	static dict_wrapper_node_ids inst;
	return inst;
}

std::string node_ids::to_name( const node_ids& enum_val){
	return dict_wrapper_node_ids::instance().to_name(enum_val);
}

node_ids node_ids::from_name( const std::string& name){
	return dict_wrapper_node_ids::instance().from_name(name);
}

std::string node_ids::name() const{
	return to_name( * this );
}

void node_ids::force_initialize(){

	static bool is_initialized = false;
	if ( is_initialized ) return;
	is_initialized = true;
	new ( const_cast<node_ids*>(&expression_statement) ) node_ids ( UINT64_C( 844424930131975 ), "expression_statement" );
	new ( const_cast<node_ids*>(&member_expression) ) node_ids ( UINT64_C( 562949953421322 ), "member_expression" );
	new ( const_cast<node_ids*>(&unary_expression) ) node_ids ( UINT64_C( 562949953421315 ), "unary_expression" );
	new ( const_cast<node_ids*>(&for_statement) ) node_ids ( UINT64_C( 844424930131977 ), "for_statement" );
	new ( const_cast<node_ids*>(&initializer) ) node_ids ( UINT64_C( 1125899906842624 ), "initializer" );
	new ( const_cast<node_ids*>(&function_type) ) node_ids ( UINT64_C( 281479271677956 ), "function_type" );
	new ( const_cast<node_ids*>(&variable_declaration) ) node_ids ( UINT64_C( 281474976710657 ), "variable_declaration" );
	new ( const_cast<node_ids*>(&cond_expression) ) node_ids ( UINT64_C( 562949953421319 ), "cond_expression" );
	new ( const_cast<node_ids*>(&case_label) ) node_ids ( UINT64_C( 1970324836974594 ), "case_label" );
	new ( const_cast<node_ids*>(&type_specifier) ) node_ids ( UINT64_C( 281479271677952 ), "type_specifier" );
	new ( const_cast<node_ids*>(&compound_statement) ) node_ids ( UINT64_C( 844424930131974 ), "compound_statement" );
	new ( const_cast<node_ids*>(&typedef_definition) ) node_ids ( UINT64_C( 281474976710658 ), "typedef_definition" );
	new ( const_cast<node_ids*>(&struct_type) ) node_ids ( UINT64_C( 281479271677955 ), "struct_type" );
	new ( const_cast<node_ids*>(&label) ) node_ids ( UINT64_C( 1970324836974592 ), "label" );
	new ( const_cast<node_ids*>(&while_statement) ) node_ids ( UINT64_C( 844424930131971 ), "while_statement" );
	new ( const_cast<node_ids*>(&program) ) node_ids ( UINT64_C( 1688849860263936 ), "program" );
	new ( const_cast<node_ids*>(&builtin_type) ) node_ids ( UINT64_C( 281479271677953 ), "builtin_type" );
	new ( const_cast<node_ids*>(&switch_statement) ) node_ids ( UINT64_C( 844424930131973 ), "switch_statement" );
	new ( const_cast<node_ids*>(&statement) ) node_ids ( UINT64_C( 844424930131968 ), "statement" );
	new ( const_cast<node_ids*>(&expression_initializer) ) node_ids ( UINT64_C( 1125899906842625 ), "expression_initializer" );
	new ( const_cast<node_ids*>(&cast_expression) ) node_ids ( UINT64_C( 562949953421316 ), "cast_expression" );
	new ( const_cast<node_ids*>(&if_statement) ) node_ids ( UINT64_C( 844424930131970 ), "if_statement" );
	new ( const_cast<node_ids*>(&parameter) ) node_ids ( UINT64_C( 281474976710659 ), "parameter" );
	new ( const_cast<node_ids*>(&constant_expression) ) node_ids ( UINT64_C( 562949953421313 ), "constant_expression" );
	new ( const_cast<node_ids*>(&node) ) node_ids ( UINT64_C( 0 ), "node" );
	new ( const_cast<node_ids*>(&variable_expression) ) node_ids ( UINT64_C( 562949953421314 ), "variable_expression" );
	new ( const_cast<node_ids*>(&dowhile_statement) ) node_ids ( UINT64_C( 844424930131972 ), "dowhile_statement" );
	new ( const_cast<node_ids*>(&ident_label) ) node_ids ( UINT64_C( 1970324836974593 ), "ident_label" );
	new ( const_cast<node_ids*>(&declaration) ) node_ids ( UINT64_C( 281474976710656 ), "declaration" );
	new ( const_cast<node_ids*>(&array_type) ) node_ids ( UINT64_C( 281479271677954 ), "array_type" );
	new ( const_cast<node_ids*>(&jump_statement) ) node_ids ( UINT64_C( 844424930131976 ), "jump_statement" );
	new ( const_cast<node_ids*>(&alias_type) ) node_ids ( UINT64_C( 281479271677957 ), "alias_type" );
	new ( const_cast<node_ids*>(&binary_expression) ) node_ids ( UINT64_C( 562949953421317 ), "binary_expression" );
	new ( const_cast<node_ids*>(&expression_list) ) node_ids ( UINT64_C( 562949953421318 ), "expression_list" );
	new ( const_cast<node_ids*>(&member_initializer) ) node_ids ( UINT64_C( 1125899906842626 ), "member_initializer" );
	new ( const_cast<node_ids*>(&declaration_statement) ) node_ids ( UINT64_C( 844424930131969 ), "declaration_statement" );
	new ( const_cast<node_ids*>(&index_expression) ) node_ids ( UINT64_C( 562949953421320 ), "index_expression" );
	new ( const_cast<node_ids*>(&declarator) ) node_ids ( UINT64_C( 281474976710661 ), "declarator" );
	new ( const_cast<node_ids*>(&null_declaration) ) node_ids ( UINT64_C( 281474976710660 ), "null_declaration" );
	new ( const_cast<node_ids*>(&identifier) ) node_ids ( UINT64_C( 1407374883553280 ), "identifier" );
	new ( const_cast<node_ids*>(&expression) ) node_ids ( UINT64_C( 562949953421312 ), "expression" );
	new ( const_cast<node_ids*>(&call_expression) ) node_ids ( UINT64_C( 562949953421321 ), "call_expression" );

}


		
node_ids::node_ids( const storage_type& val, const std::string& name ): node_ids::base_type(val){
	node_ids tmp(val);
	dict_wrapper_node_ids::instance().insert( tmp, name );
}

const node_ids node_ids::expression_statement ( UINT64_C( 844424930131975 ), "expression_statement" );
const node_ids node_ids::member_expression ( UINT64_C( 562949953421322 ), "member_expression" );
const node_ids node_ids::unary_expression ( UINT64_C( 562949953421315 ), "unary_expression" );
const node_ids node_ids::for_statement ( UINT64_C( 844424930131977 ), "for_statement" );
const node_ids node_ids::initializer ( UINT64_C( 1125899906842624 ), "initializer" );
const node_ids node_ids::function_type ( UINT64_C( 281479271677956 ), "function_type" );
const node_ids node_ids::variable_declaration ( UINT64_C( 281474976710657 ), "variable_declaration" );
const node_ids node_ids::cond_expression ( UINT64_C( 562949953421319 ), "cond_expression" );
const node_ids node_ids::case_label ( UINT64_C( 1970324836974594 ), "case_label" );
const node_ids node_ids::type_specifier ( UINT64_C( 281479271677952 ), "type_specifier" );
const node_ids node_ids::compound_statement ( UINT64_C( 844424930131974 ), "compound_statement" );
const node_ids node_ids::typedef_definition ( UINT64_C( 281474976710658 ), "typedef_definition" );
const node_ids node_ids::struct_type ( UINT64_C( 281479271677955 ), "struct_type" );
const node_ids node_ids::label ( UINT64_C( 1970324836974592 ), "label" );
const node_ids node_ids::while_statement ( UINT64_C( 844424930131971 ), "while_statement" );
const node_ids node_ids::program ( UINT64_C( 1688849860263936 ), "program" );
const node_ids node_ids::builtin_type ( UINT64_C( 281479271677953 ), "builtin_type" );
const node_ids node_ids::switch_statement ( UINT64_C( 844424930131973 ), "switch_statement" );
const node_ids node_ids::statement ( UINT64_C( 844424930131968 ), "statement" );
const node_ids node_ids::expression_initializer ( UINT64_C( 1125899906842625 ), "expression_initializer" );
const node_ids node_ids::cast_expression ( UINT64_C( 562949953421316 ), "cast_expression" );
const node_ids node_ids::if_statement ( UINT64_C( 844424930131970 ), "if_statement" );
const node_ids node_ids::parameter ( UINT64_C( 281474976710659 ), "parameter" );
const node_ids node_ids::constant_expression ( UINT64_C( 562949953421313 ), "constant_expression" );
const node_ids node_ids::node ( UINT64_C( 0 ), "node" );
const node_ids node_ids::variable_expression ( UINT64_C( 562949953421314 ), "variable_expression" );
const node_ids node_ids::dowhile_statement ( UINT64_C( 844424930131972 ), "dowhile_statement" );
const node_ids node_ids::ident_label ( UINT64_C( 1970324836974593 ), "ident_label" );
const node_ids node_ids::declaration ( UINT64_C( 281474976710656 ), "declaration" );
const node_ids node_ids::array_type ( UINT64_C( 281479271677954 ), "array_type" );
const node_ids node_ids::jump_statement ( UINT64_C( 844424930131976 ), "jump_statement" );
const node_ids node_ids::alias_type ( UINT64_C( 281479271677957 ), "alias_type" );
const node_ids node_ids::binary_expression ( UINT64_C( 562949953421317 ), "binary_expression" );
const node_ids node_ids::expression_list ( UINT64_C( 562949953421318 ), "expression_list" );
const node_ids node_ids::member_initializer ( UINT64_C( 1125899906842626 ), "member_initializer" );
const node_ids node_ids::declaration_statement ( UINT64_C( 844424930131969 ), "declaration_statement" );
const node_ids node_ids::index_expression ( UINT64_C( 562949953421320 ), "index_expression" );
const node_ids node_ids::declarator ( UINT64_C( 281474976710661 ), "declarator" );
const node_ids node_ids::null_declaration ( UINT64_C( 281474976710660 ), "null_declaration" );
const node_ids node_ids::identifier ( UINT64_C( 1407374883553280 ), "identifier" );
const node_ids node_ids::expression ( UINT64_C( 562949953421312 ), "expression" );
const node_ids node_ids::call_expression ( UINT64_C( 562949953421321 ), "call_expression" );


