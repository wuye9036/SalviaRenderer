struct type_deduce_table_entry{
	type_deduce_table_entry( const vector<h_ast_node_type>& params );

	vector<h_ast_node_type> param_types;

	size_t hash_value();
	bool operator == ( const type_deduce_table_entry& rhs );
};


struct type_deduce_result{
	// construct type_deduce_result with params_count parameters all of them type is param_type
	template< typename ParamTypeT, typename ReturnTypeT >
	static type_deduce_result create( ParamTypeT param_type, size_t params_count, ReturnTypeT ret_type ){
		type_deduce_result ret_result;
		for(size_t i_param = 0; i_param < params_count; ++i_param ){
			ret_result.add_param( param_type );
		}
		ret_result.set_return_type( ret_type );
		return ret_result;
	}

	void set_return_type( sasl_types ret_type );
	void set_return_type( h_ast_node_type ret_type );

	void add_param( sasl_types param_type ){
		add_matched_param( type_match_result::create( param_type, param_type ) );
	}

	void add_param( h_ast_node_type param_type ){
		add_matched_param( type_match_result::create( param_type, param_type ) );
	}

	bool add_matched_param( const type_matched_result& matched_param){
		if( matched_param.is_equivalence() ){
			input_types.push_back( macthed_param );
			return true;
		}

		if( matched_param.is_compatible() ){
			input_types.push_back( matched_param );
			cost += mathced_param.cost;
			return true;
		}

		if( matched_param.is_convertible() ){
			input_types.push_back( matched_param );
			cost += matched_param.cost;
			warning = compiler_warnings::type_convert_may_lost_data;
			return true;
		}
	
		input_types.clear();
		error = compiler_warnings::type_not_matched;
		cost = limits<size_t>::max;
		result_type = h_ast_node_type();
		return false;
	}


	// no warning, no error
	bool is_successful();
	// deduced but maybe with warning.
	bool is_deduced();

	vector< h_ast_node_type > src_types();
	vector< h_ast_node_type > dest_types();

	//形参和实参的类型。src为实参，dest为形参。
	vector< type_match_result > input_types;
	h_ast_node_type result_type;

	//匹配代价。
	size_t cost;
	
	compiler_warnings warning;
	compiler_errors error;

	//在推导时，左边有 lhs_fixed_count 个参数是强制相同的。用于Assignment操作符。
	//默认是0，即不进行约束。所有参数均可自由匹配。
	size_t lhs_fixed_count;
};

class deducer{
	void add_deduction( type_deduce_result result ){
		exist_deductions[ type_deduce_table_entry( result.src_types() ) ] = result;
	}

	//用于普通的函数和操作符的推导。
	type_deduce_result deduce( const vector< h_ast_node_type >& types ){
		// try to find the whole matched solution.
		type_deduce_table_entry params_entry( types );
		if( exist_deductions.has_key( params_entry ) ){
			return exist_deductions[ params_entry ];
		}

		// try to get all of aviable match, and record its cost for choosing a best match.
		type_deduce_result best_match( type_deduce_result::cannot_deduced() );
		type_deduce_result second_best_match( type_deduce_result::cannot_deduced() );
		
		typedef unordered_map< type_deduce_table_entry, type_deduce_result > deduce_table_item_t;

		BOOST_FOREACH( const deduce_table_item_t& deduction_item_pair, exists_deductions ){
			const type_deduce_result& candidate_deduction = deduction_item_pair.second;

			type_deduce_result current_match = try_match( types, candidate_deduction.src_types(), candidate_deduction.lhs_fixed_count );
			if( ! current_match.is_convertible() ){
				continue;
			}
			current_match.result_type = candidate_deduction.result_type;

			if( current_match.cost() < best_match.cost() ){
				second_best_match = best_match;
				best_match = current_match;
				continue;
			}

			if( current_match.cost() < second_match.cost() ){
				second_best_match = current_match;
			}
		}
		
		//如果匹配项代价相同，则返回二义性错误。
		if( best_match.cost() == second_best_match.cost() ){
			return type_reduce_result::ambigous( best_match, second_best_match );
		}

		//返回最佳匹配。
		return best_match;
	}

private:
	// match input types to candidate deduction input types.
	type_deduce_result try_match( vector< h_ast_node_type >& input_types, const vector< h_ast_node_type >& ref_types, size_t lhs_fixed_count ){
		type_deduce_result ret_deduction( type_deduce_result::cannot_deduced() );

		if( ref_types.size() < input_types.size() ){
			return type_deduce_result::cannot_deduced();
		}

		for( int i_type = 0; i_type < ref_types.size(); ++i_type ){
			type_conv_result conv_result = implicit_type_conversation::convert( input_types[i_type], ref_types[ i_type ] );
			if( i_type < lhs_fixed_count ){
				if( ! conv_result.is_equivalene() ){
					return type_deduce_result::cannot_deduced();
				}
			}

			if( conv_result.is_convertible() && i_type >= lhs_fixed_count ){
				if( ! ret_deduction.add_matched_param( conv_result ) ){
					return type_deduce_result::cannot_deduced();
				}
			}
		}

		return ret_deduction;
	}

	unordered_map< type_deduce_table_entry, type_deduce_result > exist_deductions;
};