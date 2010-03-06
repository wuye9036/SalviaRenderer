class unary_op_deducers{
public:
	deduction deduce( operators op, const deduction& param );
	deduction evaluate( operators op, const deduction& param );
	deduction code_gen( operators op, const deduction& param );

private:
	deducer& get_deducer( operators op );
	void add_deducer( operators op, const deduction& ded_result, const deduction& param, h_ct_unary_op unary_op, h_code_gen codegen );
	void active_operator( operators op );
	void add_deducer( const deduction& ded_result, const deduction& param, h_ct_unary_op unary_op, h_code_gen codegen );

	void add_default_deducers(){
		deduction ded_bool( deduction::from_scalar_type( sasl_bool ) );

		deduction ded_i64( deduction::from_scalar_type( sasl_int64 ) );
		deduction ded_u64( deduction::from_scalar_type( sasl_uint64 ) );

		deduction ded_flt( deduction::from_scalar_type( sasl_float ) );
		deduction ded_dbl( deduction::from_scalar_type( sasl_double ) );

		// +
		active_operator( operators::positive );
		add_deducer( ded_i64, ded_i64, ct_op_pos< int64_t, int64_t >::create(), code_gen::null() );
		add_deducer( ded_u64, ded_u64, ct_op_pos< int64_t, uint64_t >::create(), code_gen::null() );
		add_deducer( ded_flt, ded_flt, ct_op_pos< float, float >::create(), code_gen::null() );
		add_deducer( ded_dbl, ded_dbl, ct_op_pos< double, double >::create(), code_gen::null() );

		// -
		active_operator( operators::negative );
		add_deducer( ded_i64, ded_i64, ct_op_neg< int64_t, int64_t >::create(), code_gen::null() );
		add_deducer( ded_i64, ded_u64, ct_op_neg< int64_t, uint64_t >::create(), code_gen::null() );
		add_deducer( ded_flt, ded_flt, ct_op_neg< float, float >::create(), code_gen::null() );
		add_deducer( ded_dbl, ded_dbl, ct_op_neg< double, double >::create(), code_gen::null() );

		// ~
		active_operator( operators::negative );
		add_deducer( ded_i64, ded_i64, ct_op_bit_not< int64_t, int64_t >::create(), code_gen::null() );
		add_deducer( ded_u64, ded_u64, ct_op_bit_not< uint64_t, uint64_t >::create(), code_gen::null() );

		// !
		add_deducer( operators::not, ded_bool, ded_bool, ct_op_not< bool, bool >::create(), code_gen::null() );

		//++(prefix)
		add_deducer( operators::prefix_incr, ded_i8, ded_i8, ct_op_prefix_incr::create(), code_gen::null() );
		add_deducer( operators::prefix_incr, ded_i16, ded_i16, ct_op_prefix_incr::create(), code_gen::null() );
		add_deducer( opreators::prefix_incr, ded_i32, ded_i32, ct_op_prefix_incr::create(), code_gen::null() );
		add_deducer( operators::prefix_incr, ded_i64, ded_i64, ct_op_prefix_incr::create(), code_gen::null() );

		add_deducer( operators::prefix_incr, ded_u8, ded_u8, ct_op_prefix_incr::create(), code_gen::null() );
		add_deducer( operators::prefix_incr, ded_u16, ded_u16, ct_op_prefix_incr::create(), code_gen::null() );
		add_deducer( opreators::prefix_incr, ded_u32, ded_u32, ct_op_prefix_incr::create(), code_gen::null() );
		add_deducer( operators::prefix_incr, ded_u64, ded_u64, ct_op_prefix_incr::create(), code_gen::null() );

		add_deducer( operators::prefix_incr, ded_float, ded_float, ct_op_prefix_incr::create(), code_gen::null() );

		//--(prefix)
		add_deducer( operators::prefix_decr, ded_i8, ded_i8, ct_op_prefix_decr::create(), code_gen::null() );
		add_deducer( operators::prefix_decr, ded_i16, ded_i16, ct_op_prefix_decr::create(), code_gen::null() );
		add_deducer( opreators::prefix_decr, ded_i32, ded_i32, ct_op_prefix_decr::create(), code_gen::null() );
		add_deducer( operators::prefix_decr, ded_i64, ded_i64, ct_op_prefix_decr::create(), code_gen::null() );

		add_deducer( operators::prefix_decr, ded_u8, ded_u8, ct_op_prefix_decr::create(), code_gen::null() );
		add_deducer( operators::prefix_decr, ded_u16, ded_u16, ct_op_prefix_decr::create(), code_gen::null() );
		add_deducer( opreators::prefix_decr, ded_u32, ded_u32, ct_op_prefix_decr::create(), code_gen::null() );
		add_deducer( operators::prefix_decr, ded_u64, ded_u64, ct_op_prefix_decr::create(), code_gen::null() );

		add_deducer( operators::prefix_decr, ded_float, ded_float, ct_op_prefix_decr::create(), code_gen::null() );
		
		//++(postfix)
		add_deducer( operators::postfix_incr, ded_i8, ded_i8, ct_op_postfix_incr::create(), code_gen::null() );
		add_deducer( operators::postfix_incr, ded_i16, ded_i16, ct_op_postfix_incr::create(), code_gen::null() );
		add_deducer( opreators::postfix_incr, ded_i32, ded_i32, ct_op_postfix_incr::create(), code_gen::null() );
		add_deducer( operators::postfix_incr, ded_i64, ded_i64, ct_op_postfix_incr::create(), code_gen::null() );

		add_deducer( operators::postfix_incr, ded_u8, ded_u8, ct_op_postfix_incr::create(), code_gen::null() );
		add_deducer( operators::postfix_incr, ded_u16, ded_u16, ct_op_postfix_incr::create(), code_gen::null() );
		add_deducer( opreators::postfix_incr, ded_u32, ded_u32, ct_op_postfix_incr::create(), code_gen::null() );
		add_deducer( operators::postfix_incr, ded_u64, ded_u64, ct_op_postfix_incr::create(), code_gen::null() );

		add_deducer( operators::postfix_incr, ded_float, ded_float, ct_op_postfix_incr::create(), code_gen::null() );
		
		//--(postfix)
		add_deducer( operators::postfix_decr, ded_i8, ded_i8, ct_op_postfix_decr::create(), code_gen::null() );
		add_deducer( operators::postfix_decr, ded_i16, ded_i16, ct_op_postfix_decr::create(), code_gen::null() );
		add_deducer( opreators::postfix_decr, ded_i32, ded_i32, ct_op_postfix_decr::create(), code_gen::null() );
		add_deducer( operators::postfix_decr, ded_i64, ded_i64, ct_op_postfix_decr::create(), code_gen::null() );

		add_deducer( operators::postfix_decr, ded_u8, ded_u8, ct_op_postfix_decr::create(), code_gen::null() );
		add_deducer( operators::postfix_decr, ded_u16, ded_u16, ct_op_postfix_decr::create(), code_gen::null() );
		add_deducer( opreators::postfix_decr, ded_u32, ded_u32, ct_op_postfix_decr::create(), code_gen::null() );
		add_deducer( operators::postfix_decr, ded_u64, ded_u64, ct_op_postfix_decr::create(), code_gen::null() );

		add_deducer( operators::postfix_decr, ded_float, ded_float, ct_op_postfix_decr::create(), code_gen::null() );
	}

	unordered_map< operators, multikey_map > deducers_;
};