// the result of type matching. it can be used in type conversation.
struct type_match_result{
	type_match_result create( sasl_types lhs, sasl_types rhs );
	type_match_result create( h_ast_node_type src_type, h_ast_node_type dest_type );

	h_ast_node_type src_type;
	h_ast_node_type dest_type;

	//not need type conversation.
	bool is_equivalence(){
		if( src_type == dest_type ){
			return true;
		}
	}
	
	//converted without any warning and error.
	bool is_compatible();

	//type can converted but maybe with some warnings.
	bool is_convertable();

	size_t cost();
	compiler_warnings warning;
	compiler_errors error;
};

struct implicit_type_conversation{
	type_conv_result type_convert( h_ast_node_type src_type, h_ast_node_type dest_type );
};