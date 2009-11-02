class implicit_typecast_deducers{
	deducers()
};

class member_deducer{
	type_deduce_result deduce( h_ast_node_type obj_type, h_ast_node_identifier member_name );
};

class unary_operator_deducers{
	type_deduce_result deduce( operators op, h_ast_node_type param_type ){
	}
	deducer& deducer( operators op ){
		return deducers_[op];
	}

private:
	void initialized_deducers();
	unorded_map< operators, deducer > deducers_;
};

class binary_operator_deducers{
	type_deduce_result deduce( operators op, h_ast_node_type lhs_type, h_ast_node_type rhs_type ){
		//vector< h_ast_node_type > deducer
		return deducers_[op].deduce( lhs_type, rhs_type );
	}
	deducer& deducer( operators op ){
		return deducers_[op];
	}
private:
	deducer& try_get_deducer( operators op ){
		if( ! deducers_.has_key(op) ){
			deducer empty_deducer;
			deducers_[op] = empty_deducer;	
		}
		return deducers_[op];
	}

	void initialize_deducers(){
		initialize_equal_deducer();
		initialize_bit_assign_deducer();
		initialize_shift_assign_deducer();
		initialize_div_assign_deducer();
		initialize_mul_assign_deducer();
		initialize_add_sub_assign_deducer();
		initialize_bitop_deducer();
		initialize_shift_deducer();
		initialize_rel_deducer();
		initialize_mul_deducer();
		initialize_div_deducer();
		initialize_sub_deducer();
		initialize_add_deducer();
	}

	void initialize_equal_deducer(){
		deducer& deducer_eq( try_get_deducer( operators::equal ) );
		deducer& deducer_ne( try_get_deducer( operators::not_equal ) );

		add_eq_deduce_results( deducer_eq );
		add_eq_deduce_results( deducer_ne );
	}
	void initialize_bit_assign_deducer(){
		reducer& reducer_bitand_assign( try_get_reducer( operators::bit_and_assign ) );
		reducer& reducer_bitor_assign( try_get_reducer( operators::bit_or_assign ) );
		reducer& reducer_bitxor_assign( try_get_reducer( operators::bit_xor_assign ) );

		add_integer_bitop_reduce_results( reducer_bitand_assign, 1 );
		add_integer_bitop_reduce_reuslts( reducer_bitor_assign, 1 );
		add_integer_bitop_reduce_results( reducer_bitxor_assign, 1 );
	}

	void initialize_shift_assign_deducer(){
		deducer& deducer_ls_assign( try_get_deducer( operators::lshift_assign ) );
		deducer& deducer_rs_assign( try_get_deducer( operators::rshift_assign ) );

		add_integer_shift_deduce_results( deducer_ls_assign );
		add_integet_shift_deduce_results( deducer_rs_assign );
	}

	void initialize_div_assign_deducer(){
		deducer& deducer_div_assign( try_get_deducer( operators::div_assign ) );
		add_scalar_arithmetic_deduce_result( deducer_div_assign, 1 );
		add_vec_div_scalar_deduce_results( deducer_div_assign, 1 );
		add_mat_div_scalar_deduce_results( deducer_div_assign, 1 );
	}

	void initialize_mul_assign_deducer(){
		deducer& deducer_mul_assign( try_get_deducer( operators::mul_assign ) );
		add_scalar_arithmetic_deduce_result( deducer_mul_assign, 1 );
		add_scalar_mul_vec_deduce_results( deducer_mul_assign, 1 );
		add_scalar_mul_mat_deduce_results( deducer_mul_assign, 1 );
	}

	void initialize_add_sub_assign_deducer(){
		deducer& deducer_add_assign( try_get_deducer( operators::add_assign ) );
		deducer& deducer_sub_assign( try_get_deducer( operators::sub_assign ) );

		construct_scalar_arithmetic_deduce_results( deducer_add_assign, 1 );
		construct_scalar_arithmetic_deduce_results( deducer_sub_assign, 1 );

		// vector
		for( size_t i_dim = 2; i_dim <= 4; ++i_dim ){
			BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
				type_deduce_result deduction(
					type_deduce_result::create( 
					ast_node_vector_type::create( scalar_type, i_dim ),
					2,
					ast_node_vector_type::create( scalar_type, i_dim )
					)
					);

				deduction.lhs_fixed_count = 1;
				deducer_add.add_deduction( deduction );
				deducer_sub.add_deduction( deduction );
			}
		}
		
		//matrix
		for( size_t row_dim = 1; row_dim <= 4; ++row_dim ){
			for( size_t col_dim = 1; col_dim <= 4; ++col_dim ){
				BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
					type_deduce_result deduction(
						type_deduce_result::create( 
						ast_node_matrix_type::create( scalar_type, row_dim, col_dim ),
						2,
						ast_node_matrix_type::create( scalar_type, row_dim, col_dim )
						)
					);
					deduction.lhs_fixed_count = 1;
					deducer_add.add_deduction( deduction );
					deducer_sub.add_deduction( deduction );
				}
			}
		}
	}

	void initialize_bitop_deducer(){
		reducer& reducer_bitand( try_get_reducer( operators::bit_and ) );
		reducer& reducer_bitor( try_get_reducer( operators::bit_or ) );
		reducer& reducer_bitxor( try_get_reducer( operators::bit_xor ) );

		add_integer_bitop_reduce_results( reducer_bitand );
		add_integer_bitop_reduce_reuslts( reducer_bitor );
		add_integer_bitop_reduce_results( reducer_bitxor );
	}
	void initialize_shift_deducer(){
		deducer& deducer_ls( try_get_deducer( operators::left_shift ) );
		deducer& deducer_rs( try_get_deducer( operators::right_shift ) );

		add_integer_shift_deduce_results( deducer_ls );
		add_integet_shift_deduce_results( deducer_rs );
	}
	void initialize_rel_deducer(){
		deducer& deducer_lt( try_get_deducer( operators::less ) );
		deducer& deducer_le( try_get_deducer( operators::less_equal ) );
		deducer& deducer_ge( try_get_deducer( operators::greater_equal ) );
		deducer& deducer_gt( try_get_deducer( operators::greater ) );

		add_scalar_relation_deduce_result( deducer_lt );
		add_scalar_relation_deduce_result( deducer_le );
		add_scalar_relation_deduce_result( deducer_ge );
		add_scalar_relation_deduce_result( deducer_gt );
	}

	void initialize_mul_deducer(){
		deducer& deducer_mul( try_get_deducer( operators::mul ) );
		add_scalar_arithmetic_deduce_result( deducer_mul );
		add_scalar_mul_vec_deduce_results( deducer_mul );
		add_scalar_mul_mat_deduce_results( deducer_mul );
	}
	void initialize_div_deducer(){
		deducer& deducer_div( try_get_deducer( operators::div ) );
		add_scalar_arithmetic_deduce_result( deducer_div );
		add_vec_div_scalar_deduce_results( deducer_div );
		add_mat_div_scalar_deduce_results( deducer_div );
	}

	void initialize_sub_deducer(){
		deducer& deducer_sub( try_get_deducer( operators::sub ) );
		construct_scalar_arithmetic_deduce_result( deducer_sub );

		// vector
		for( size_t i_dim = 2; i_dim <= 4; ++i_dim ){
			BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
				deducer_sub.add_deduction( 
					type_deduce_result::create( 
					ast_node_vector_type::create( scalar_type, i_dim ),
					2,
					ast_node_vector_type::create( scalar_type, i_dim )
					)
				);
			}
		}
		
		//matrix
		for( size_t row_dim = 1; row_dim <= 4; ++row_dim ){
			for( size_t col_dim = 1; col_dim <= 4; ++col_dim ){
				BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
					deducer_sub.add_deduction( 
						type_deduce_result::create( 
						ast_node_matrix_type::create( scalar_type, row_dim, col_dim ),
						2,
						ast_node_matrix_type::create( scalar_type, row_dim, col_dim )
						)
					);
				}
			}
		}
	}

	void initialize_add_deducer(){
		//add defaule operator + deducers
		deducer& deducer_add( try_get_deducer( operators::add ) );

		// scalar
		add_scalar_arithmetic_deduce_results( deducer_add );

		// vector
		for( size_t i_dim = 2; i_dim <= 4; ++i_dim ){
			BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
				deducer_add.add_deduction( 
					type_deduce_result::create( 
					ast_node_vector_type::create( scalar_type, i_dim ),
					2,
					ast_node_vector_type::create( scalar_type, i_dim )
					)
				);
			}
		}
		
		//matrix
		for( size_t row_dim = 1; row_dim <= 4; ++row_dim ){
			for( size_t col_dim = 1; col_dim <= 4; ++col_dim ){
				BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
					deducer_add.add_deduction( 
						type_deduce_result::create( 
						ast_node_matrix_type::create( scalar_type, row_dim, col_dim ),
						2,
						ast_node_matrix_type::create( scalar_type, row_dim, col_dim )
						)
					);
				}
			}
		}
				
	}

	void add_scalar_arithmetic_deduce_results( deducer& ded, size_t lhs_fixed_count = 0 ){
		BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
			type_deduce_result deduction( type_deduce_result::create( scalar_type, 2, scalar_type ) );
			deduction.lhs_fixed_count = lhs_fixed_count;
			ded.add_deduction( deduction );
		}
	}
	void add_scalar_relation_deduce_results( deducer& ded, size_t lhs_fixed_count  = 0 ){
		BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
			type_deduce_result deduction( type_deduce_result::create( scalar_type, 2, sasl_types::sasl_bool ) );
			deduction.lhs_fixed_count = lhs_fixed_count;
			ded.add_deduction( deduction );
		}
	}
	void add_integer_bitop_deduce_results( deducer& ded, size_t lhs_fixed_count = 0 ){
		BOOST_FOREACH( sasl_types integer_type, sasl_integer_types() ){
			type_deduce_result deduction( type_deduce_result::create( integer_type, 2, integer_type ) );
			deduction.lhs_fixed_count = lhs_fixed_count;
			ded.add_deduction( deduction );
		}
	}
	void add_integer_shift_deduce_results( deducer& ded, size_t lhs_fixed_count = 0 ){
		BOOST_FOREACH( sasl_types integer_type, sasl_types ){
			BOOST_FOREACH( sasl_types unsigned_type, sasl_unsigned_int_types ){
				type_deduce_result deduction;
				deduction.add_param( integer_type );
				deduction.add_param( unsigned_type );
				deduction.set_result_type( integer_type );
				deduction.lhs_fixed_count = lhs_fixed_count;

				ded.add_deduction( deduction );
			}
		}
	}

	void add_scalar_mul_vec_deduce_results( deducer& ded, size_t lhs_fixed_count = 0){
		for( size_t i_dim = 2; i_dim <= 4; ++i_dim ){
			BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
				type_deduce_result pos_deduction;
				type_deduce_result neg_deduction;
				h_ast_node_vector_type vec_type( ast_node_vector_type::create( scalar_type, i_dim ) );

				pos_deduction.add_param( vec_type );
				pos_deduction.add_param( scalar_type );
				pos_deduction.set_result_type( vec_type );
				
				neg_deduction.add_param( scalar_type );
				neg_deduction.add_param( vec_type );
				neg_deduction.set_reuslt_type( vec_type );
				
				pos_deduction.lhs_fixed_count = neg_deduction.lhs_fixed_count = lhs_fixed_count;

				deducer.add_deduction( pos_deduction );
				deducer.add_deduction( neg_deduction );
			}
		}	
	}

	void add_scalar_mul_mat_deduce_results( deducer& ded, size_t lhs_fixed_count = 0 ){
		//matrix
		for( size_t row_dim = 1; row_dim <= 4; ++row_dim ){
			for( size_t col_dim = 1; col_dim <= 4; ++col_dim ){
				BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){

					type_deduce_result pos_deduction;
					type_deduce_result neg_deduction;
					h_ast_node_matrix_type mat_type( ast_node_matrix_type::create( scalar_type, row_dim, col_dim ) );

					pos_deduction.add_param( mat_type );
					pos_deduction.add_param( scalar_type );
					pos_deduction.set_result_type( mat_type );
					
					neg_deduction.add_param( scalar_type );
					neg_deduction.add_param( mat_type );
					neg_deduction.set_reuslt_type( mat_type );
					
					pos_deduction.lhs_fixed_count = neg_deduction.lhs_fixed_count = lhs_fixed_count;

					deducer.add_deduction( pos_deduction );
					deducer.add_deduction( neg_deduction );
				}
			}
		}
	}
	void add_vec_div_scalar_deduce_results( deducer& ded, size_t lhs_fixed_count = 0 ){
		for( size_t i_dim = 2; i_dim <= 4; ++i_dim ){
			BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
				type_deduce_result deduction;
				h_ast_node_vector_type vec_type( ast_node_vector_type::create( scalar_type, i_dim ) );
				deduction.add_param( vec_type );
				deduction.add_param( scalar_type );
				deduction.set_result_type( vec_type );
				deduction.lhs_fixed_count = lhs_fixed_count;
				deducer.add_deduction( deduction );
			}
		}	
	}

	void add_mul_div_scalar_deduce_results( deducer& ded, size_t lhs_fixed_count = 0 ){
		for( size_t row_dim = 1; row_dim <= 4; ++row_dim ){
			for( size_t col_dim = 1; col_dim <= 4; ++col_dim ){
				BOOST_FOREACH( sasl_types scalar_type, sasl_scalar_types ){
					type_deduce_result deduction;
					h_ast_node_vector_type mat_type( ast_node_matrix_type::create( scalar_type, row_dim, col_dim ) );
					deduction.add_param( mat_type );
					deduction.add_param( scalar_type );
					deduction.set_result_type( mat_type );
					deduction.lhs_fixed_count = lhs_fixed_count;
					deducer.add_deduction( deduction );
				}
			}
		}
	}

	void add_eq_deduce_results( deducer&  ded, size_t lhs_fixed_count = 0 {

		BOOST_FOREACH( sasl_types single_component_type, sasl_single_component_types() ){
			//scalar
			type_deduce_result deduction = type_deduce_result::create( sasl_types::sasl_bool, 2, sasl_types::sasl_bool );
			deduction.lhs_fixed_count = lhs_fixed_count;
			ded.add_deduction( deduction );

			//vector
			for( size_t i_dim = 2; i_dim <= 4; ++i_dim){
				h_ast_node_vector_type vec_type = h_ast_node_vector_type::create( single_component_type, i_dim );
				deduction = type_deduce_result::create( vec_type, 2, sasl_types::sasl_bool );
				deduction.lhs_fixed_count = lhs_fixed_count;
				ded.add_deduction( deduction );
			}

			//matrix
			for( size_t row_dim = 1; row_dim <= 4; ++row_dim ){
				for( size_t col_dim = 1; col_dim <= 4; ++col_dim ){
					h_ast_node_matrix_type mat_type( ast_node_matrix_type::create( scalar_type, row_dim, col_dim ) );
					deduction = type_deduce_result::create( mat_type, 2, sasl_types::sasl_bool );
					deduction.lhs_fixed_count = lhs_fixed_count;
					ded.add_deduction( deduction );
				}
			}
		}
	}

	static const vector< sasl_types >& sasl_single_component_types(){
		if ( sasl_single_component_types_.empty() ){
			sasl_single_component_types_.push_back( sasl_types::sasl_bool );

			sasl_single_component_types_.push_back( sasl_types::sasl_int8 );
			sasl_single_component_types_.push_back( sasl_types::sasl_int16 );
			sasl_single_component_types_.push_back( sasl_types::sasl_int32 );
			sasl_single_component_types_.push_back( sasl_types::sasl_int64 );

			sasl_single_component_types_.push_back( sasl_types::sasl_uint8 );
			sasl_single_component_types_.push_back( sasl_types::sasl_uint16 );
			sasl_single_component_types_.push_back( sasl_types::sasl_uint32 );
			sasl_single_component_types_.push_back( sasl_types::sasl_uint64 );

			sasl_single_component_types_.push_back( sasl_types::sasl_float );
			sasl_single_component_types_.push_back( sasl_types::sasl_double );
		}

		return sasl_single_component_types_;
	}

	static const vector< sasl_types >& sasl_scalar_types(){
		if ( sasl_scalar_types_.empty() ){
			sasl_scalar_types_.push_back( sasl_types::sasl_int8 );
			sasl_scalar_types_.push_back( sasl_types::sasl_int16 );
			sasl_scalar_types_.push_back( sasl_types::sasl_int32 );
			sasl_scalar_types_.push_back( sasl_types::sasl_int64 );

			sasl_scalar_types_.push_back( sasl_types::sasl_uint8 );
			sasl_scalar_types_.push_back( sasl_types::sasl_uint16 );
			sasl_scalar_types_.push_back( sasl_types::sasl_uint32 );
			sasl_scalar_types_.push_back( sasl_types::sasl_uint64 );

			sasl_scalar_types_.push_back( sasl_types::sasl_float );
			sasl_scalar_types_.push_back( sasl_types::sasl_double );
		}

		return sasl_scalar_types_;
	}
	static const vector< sasl_types >& sasl_integer_types(){
		if( sasl_integer_types_.empty() ){
			sasl_integer_types_.push_back( sasl_types::sasl_int8 );
			sasl_integer_types_.push_back( sasl_types::sasl_int16 );
			sasl_integer_types_.push_back( sasl_types::sasl_int32 );
			sasl_integer_types_.push_back( sasl_types::sasl_int64 );

			sasl_integer_types_.push_back( sasl_types::sasl_uint8 );
			sasl_integer_types_.push_back( sasl_types::sasl_uint16 );
			sasl_integer_types_.push_back( sasl_types::sasl_uint32 );
			sasl_integer_types_.push_back( sasl_types::sasl_uint64 );
		}
		return sasl_integer_types_;
	}

	static const vector< sasl_types >& sasl_unsigned_int_types(){
		if( sasl_unsinged_int_types_.empty() ){
			sasl_unsinged_int_types_.push_back( sasl_types::sasl_int8 );
			sasl_unsinged_int_types_.push_back( sasl_types::sasl_int16 );
			sasl_unsinged_int_types_.push_back( sasl_types::sasl_int32 );
			sasl_unsinged_int_types_.push_back( sasl_types::sasl_int64 );
		}
		return sasl_unsinged_int_types_;
	}

	unordered_map< operators, deducer > deducers_;

	static vector< sasl_types > sasl_scalar_types_;
	static vector< sasl_types > sasl_integer_types_;
	static vector< sasl_types > sasl_unsinged_int_types_;
	static vector< sasl_types > sasl_single_component_types_;
};

//类型推导的堆栈机
class type_deducer{
	void push_type( h_ast_node_type );
	bool unary_deduce( operators op ){

	}
	bool binary_deduce( operators op );
	bool function_call_deduce( size_t param_count );
	bool member_deduce();
	bool index_deduce();
	bool type_cast_deduce();
};