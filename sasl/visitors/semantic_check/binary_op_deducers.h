class binop_deducers{
public:
	deduction deduce( operators op, const deduction& lhs, const deduction& rhs );
	deduction evaluate( operators op, const deduction& lhs, const deduction& rhs );
	deduction gen_code( operators op, const deduction& lhs, const deduction& rhs );
	
	deduction& add_deduction( operators op, const deduction& ref_deduction, const deduction& lhs, const deduction& rhs, h_ct_binop ct_op, h_code_gen codegen ){
		vector< h_ast_node_type > param_types
			= list_of( lhs.type )( rhs.type );
		get_deducer(op).add_deduction( param_types, ref_deduction, ct_op, codegen );
	}

private:
	unordered_map< operators, deducer > deducer_tbl_;
	deducer* p_active_deducer_;
	
	deducer& get_deducer( opeartors op ){
		if ( deducers_tbl_.find( op ) == deducers_tbl_.end() ){
			deducers_tbl_[ op ] = deducer();
		}
		return deducers_tbl_[ op ];
	}

	void active_operator( operators op ){
		p_active_deducer_ = addressof( get_deducer( op ) );
	}
	
	deduction& add_deduction( const deduction& ref_deduction, const deduction& lhs, const deduction& rhs, h_ct_binop ct_op, h_code_gen codegen ){
		vector< h_ast_node_type > param_types
			= list_of( lhs.type )( rhs.type );
		p_active_deducer_->add_deduction( param_types, ref_deduction, ct_op, codegen );
	}

	// deudcers for scalar
	void add_default_deducers(){
		deduction ded_bool( deduction::from_scalar_type( sasl_bool ) );

		deduction ded_i8( deduction::from_scalar_type( sasl_int8 ) );
		deduction ded_i16( deduction::from_scalar_type( sasl_int16 ) );
		deduction ded_i32( deduction::from_scalar_type( sasl_int32 ) );
		deduction ded_i64( deduction::from_scalar_type( sasl_int64 ) );
		deduction ded_u8( deduction::from_scalar_type( sasl_uint8 ) );
		deduction ded_u16( deduction::from_scalar_type( sasl_uint16 ) );
		deduction ded_u32( deduction::from_scalar_type( sasl_uint32 ) );
		deduction ded_u64( deduction::from_scalar_type( sasl_uint64 ) );

		deduction ded_flt( deduction::from_scalar_type( sasl_float ) );
		deduction ded_dbl( deduction::from_scalar_type( sasl_double ) );

		// +
		active_operator( operators::add );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_add<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_add<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_add<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_u64, ded_i64, ct_op_add<int64_t, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_flt, ded_flt, ded_flt, ct_op_add< float, float, float >::create(), code_gen::null() );
		add_deduction( ded_dbl, ded_dbl, ded_dbl, ct_op_add< double, double, double >::create(), code_gen::null() );

		// -
		active_operators( operators::sub );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_sub<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_sub<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_sub<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_u64, ded_i64, ct_op_sub<int64_t, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_flt, ded_flt, ded_flt, ct_op_sub< float, float, float >::create(), code_gen::null() );
		add_deduction( ded_dbl, ded_dbl, ded_dbl, ct_op_sub< double, double, double >::create(), code_gen::null() );

		// *
		active_operators( operators::mul );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_mul<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_mul<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_mul<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_u64, ded_i64, ct_op_mul<int64_t, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_flt, ded_flt, ded_flt, ct_op_mul< float, float, float >::create(), code_gen::null() );
		add_deduction( ded_dbl, ded_dbl, ded_dbl, ct_op_mul< double, double, double >::create(), code_gen::null() );

		// /
		active_operator( operators::div );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_div<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_div<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_div<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_u64, ded_i64, ct_op_div<int64_t, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_flt, ded_flt, ded_flt, ct_op_div< float, float, float >::create(), code_gen::null() );
		add_deduction( ded_dbl, ded_dbl, ded_dbl, ct_op_div< double, double, double >::create(), code_gen::null() );

		// && ||
		add_deduction( operators::and, ded_bool, ded_bool, ded_bool, ct_op_and<bool, bool, bool>::create(), code_gen::null() );
		add_deduction( operators::or, ded_bool, ded_bool, ded_bool, ct_op_or<bool, bool, bool>::create(), code_gen::null() );
		
		// & | ^
		active_operator( operators::bit_and );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_bit_and<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_bit_and<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_bit_and<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_u64, ded_i64, ct_op_bit_and<int64_t, uint64_t, int64_t>::create(), code_gen::null() );

		active_operator( operators::bit_or );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_bit_or<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_bit_or<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_bit_or<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_u64, ded_i64, ct_op_bit_or<int64_t, uint64_t, int64_t>::create(), code_gen::null() );

		active_operator( operators::bit_xor );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_bit_xor<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_bit_xor<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_bit_xor<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_u64, ded_i64, ct_op_bit_xor<int64_t, uint64_t, int64_t>::create(), code_gen::null() );

		// <
		active_operator( operators::less );
		add_deduction( ded_bool, ded_i64, ded_i64, ct_op_lt<bool, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_u64, ct_op_lt<bool, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_u64, ct_op_lt<bool, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_i64, ct_op_lt<bool, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_flt, ded_flt, ct_op_lt<bool, float, float>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_dbl, ded_dbl, ct_op_lt<bool, double, double >::create(), code_gen::null() );

		// <=
		active_operator( operators::less_equal );
		add_deduction( ded_bool, ded_i64, ded_i64, ct_op_le<bool, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_u64, ct_op_le<bool, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_u64, ct_op_le<bool, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_i64, ct_op_le<bool, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_flt, ded_flt, ct_op_le<bool, float, float>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_dbl, ded_dbl, ct_op_le<bool, double, double >::create(), code_gen::null() );

		// ==
		active_operator( operators::equal );
		add_deduction( ded_bool, ded_bool, ded_bool, ct_op_eq<bool, bool, bool>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_i64, ct_op_eq<bool, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_u64, ct_op_eq<bool, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_u64, ct_op_eq<bool, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_i64, ct_op_eq<bool, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_flt, ded_flt, ct_op_eq<bool, float, float>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_dbl, ded_dbl, ct_op_eq<bool, double, double >::create(), code_gen::null() );

		// !=
		active_operator( operators::not_equal );
		add_deduction( ded_bool, ded_bool, ded_bool, ct_op_ne<bool, bool, bool>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_i64, ct_op_ne<bool, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_u64, ct_op_ne<bool, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_u64, ct_op_ne<bool, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_i64, ct_op_ne<bool, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_flt, ded_flt, ct_op_ne<bool, float, float>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_dbl, ded_dbl, ct_op_ne<bool, double, double >::create(), code_gen::null() );

		// >=
		active_operator( operators::greater_equal );
		add_deduction( ded_bool, ded_i64, ded_i64, ct_op_ge<bool, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_u64, ct_op_ge<bool, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_u64, ct_op_ge<bool, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_i64, ct_op_ge<bool, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_flt, ded_flt, ct_op_ge<bool, float, float>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_dbl, ded_dbl, ct_op_ge<bool, double, double >::create(), code_gen::null() );

		// >
		active_operator( operators::greater );
		add_deduction( ded_bool, ded_i64, ded_i64, ct_op_gt<bool, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_u64, ct_op_gt<bool, uint64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_i64, ded_u64, ct_op_gt<bool, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_u64, ded_i64, ct_op_gt<bool, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_flt, ded_flt, ct_op_gt<bool, float, float>::create(), code_gen::null() );
		add_deduction( ded_bool, ded_dbl, ded_dbl, ct_op_gt<bool, double, double >::create(), code_gen::null() );

		// << >>
		active_operator( operators::left_shift );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_ls<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_ls<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_i64, ct_op_ls<uint64_t, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_ls<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );

		active_operator( operators::right_shift );
		add_deduction( ded_i64, ded_i64, ded_i64, ct_op_rs<int64_t, int64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_i64, ded_i64, ded_u64, ct_op_rs<int64_t, int64_t, uint64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_i64, ct_op_rs<uint64_t, uint64_t, int64_t>::create(), code_gen::null() );
		add_deduction( ded_u64, ded_u64, ded_u64, ct_op_rs<uint64_t, uint64_t, uint64_t>::create(), code_gen::null() );
	}

	// deducers for vector

	// deducers for matrix
	
};