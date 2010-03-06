//类型转换的deducers
class typecast_deducer{
public:
	deduction deduce( const deduction& ded_from, const deduction& ded_to, bool is_explicit = false ){
		// 类型推导直接返回查询表就OK。
		
	}
	deduction evaluate( const deduction& ded_from, const deduction& ded_to, bool is_explicit = false ){
		// 常量计算需要对deduction附加约束，例如超限值约束。
	}
	deduction gen_code( const deduction& ded_from, const deduction& ded_to, bool is_explicit = false ){
		// 此处仅进行必要的代码生成，不参与约束。
	}
	
	deducer_value generic_deduce( const dedeuction& ded_from, const deduction& ded_to, bool is_explicit ){
		
	}
private:
	template< typename HValueOpT, typename HCodeGenT >
	deduction& add_deduction( deduction ded_result, const deduction& ded_from, const deduction& ded_to, HValueOpT valop, HCodeGenT codegen );

	// it add conversation if ded_from.type is same as ded_to.type
	void add_default_scalar_deductions(){

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

		// convert from i8
		{
			add_deduction( ded_i16, ded_i8, ded_i16, typeconv_op< int8_t, int16_t >::create(), code_generator::null() )
				.small_to_large( 1 );
			add_deduction( ded_i32, ded_i8, ded_i32, typeconv_op< int8_t, int32_t >::create(), code_generator::null() )
				.small_to_large( 2 );
			add_deduction( ded_i64, ded_i8, ded_i64, typeconv_op< int8_t, int64_t >::create(), code_generator::null() )
				.small_to_large( 3 );

			add_deduction( ded_u8, ded_i8, ded_u8, typeconv_op< int8_t, uint8_t >::create(), code_generator::null() )
				.change_sign( 1 );
			add_deduction( ded_u16, ded_i8, ded_u16, typeconv_op< int8_t, uint16_t >::create(), code_generator::null() )
				.change_sign( 2 );
			add_deduction( ded_u32, ded_i8, ded_u32, typeconv_op< int8_t, uint32_t >::create(), code_generator::null( ) )
				.change_sign(3);
			add_deduction( ded_u64, ded_i8, ded_u64, typeconv_op< int8_t, uint64_t >::create(), code_generator::null() )
				.change_sign( 4 );
			
			add_deduction( ded_flt, ded_i8, ded_flt, typeconv_op< int8_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_i8, ded_dbl, typeconv_op< int8_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_i8, ded_bool, typeconv_op< int8_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}

		// convert from i16
		{			
			add_deduction( ded_i8, ded_i16, ded_i8, typeconv_op< int16_t, int8_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i32, ded_i16, ded_i32, typeconv_op< int16_t, int32_t>::create(), code_generator::null() )
				.samll_to_large();
			add_deduction( ded_i64, ded_i16, ded_i64, typeconv_op< int16_t, in64_t>::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_u8, ded_i16, ded_u8, typeconv_op< int16_t, uint8_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u16, ded_i16, ded_u16, typeconv_op< int16_t, uint16_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u32, ded_i16, ded_u32, typeconv_op< int16_t, uint32_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u64, ded_i16, ded_u64, typeconv_op< int16_t, uint64_t >::create(), code_generator::null() )
				.change_sign();

			add_deduction( ded_flt, ded_i16, ded_flt, typeconv_op< int16_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_i16, ded_dbl, typeconv_op< int16_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_i16, ded_bool, typeconv_op< int16_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from i32
		{
			add_deduction( ded_i8, ded_i32, ded_i8, typeconv_op< int32_t, int8_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i16, ded_i32, ded_i16, typeconv_op< int32_t, int16_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i64, ded_i32, ded_i64, typeconv_op< int32_t, int64_t>::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_u8, ded_i32, ded_u8, typeconv_op< int32_t, uint8_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u16, ded_i32, ded_u16, typeconv_op< int32_t, uint16_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u32, ded_i32, ded_u32, typeconv_op< int32_t, uint32_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u64, ded_i32, ded_u64, typeconv_op< int32_t, uint64_t >::create(), code_generator::null() )
				.change_sign();

			add_deduction( ded_flt, ded_i32, ded_flt, typeconv_op< int32_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_i32, ded_dbl, typeconv_op< int32_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_i32, ded_bool, typeconv_op< int32_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from i64
		{
			add_deduction( ded_i8, ded_i64, ded_i8, typeconv_op< int64_t, int8_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i16, ded_i64, ded_i16, typeconv_op< int64_t, int16_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i32, ded_i64, ded_i32, typeconv_op< int64_t, int32_t>::create(), code_generator::null() )
				.large_to_small();

			add_deduction( ded_u8, ded_i64, ded_u8, typeconv_op< int64_t, uint8_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u16, ded_i64, ded_u16, typeconv_op< int64_t, uint16_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u32, ded_i64, ded_u32, typeconv_op< int64_t, uint32_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_u64, ded_i64, ded_u64, typeconv_op< int64_t, uint64_t >::create(), code_generator::null() )
				.change_sign();

			add_deduction( ded_flt, ded_i64, ded_flt, typeconv_op< int64_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_i64, ded_dbl, typeconv_op< int64_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_i64, ded_bool, typeconv_op< int64_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from u8
		{
			add_deduction( ded_u16, ded_u8, ded_u16, typeconv_op< uint8_t, int16_t>::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_u32, ded_u8, ded_u32, typeconv_op< uint8_t, uint32_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_u64, ded_u8, ded_u64, typeconv_op< uint8_t, uint64_t>::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_i8, ded_u8, ded_i8, typeconv_op< uint8_t, int8_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i16, ded_u8, ded_i16, typeconv_op< uint8_t, int16_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_i32, ded_u8, ded_i32, typeconv_op< uint8_t, int32_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_i64, ded_u8, ded_i64, typeconv_op< uint8_t, int64_t >::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_flt, ded_u8, ded_flt, typeconv_op< uint8_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_u8, ded_dbl, typeconv_op< uint8_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_u8, ded_bool, typeconv_op< uint8_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from u16
		{
			add_deduction( ded_u8, ded_u16, ded_u8, typeconv_op< uint16_t, int8_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u32, ded_u16, ded_u32, typeconv_op< uint16_t, uint32_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_u64, ded_u16, ded_u64, typeconv_op< uint16_t, uint64_t>::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_i8, ded_u16, ded_i8, typeconv_op< uint16_t, int8_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i16, ded_u16, ded_i16, typeconv_op< uint16_t, int16_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i32, ded_u16, ded_i32, typeconv_op< uint16_t, int32_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_i64, ded_u16, ded_i64, typeconv_op< uint16_t, int64_t >::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_flt, ded_u16, ded_flt, typeconv_op< uint16_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_u16, ded_dbl, typeconv_op< uint16_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_u16, ded_bool, typeconv_op< uint16_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from u32
		{
			add_deduction( ded_u8, ded_u32, ded_u8, typeconv_op< uint32_t, int8_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u16, ded_u32, ded_u16, typeconv_op< uint32_t, uint16_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u64, ded_u32, ded_u64, typeconv_op< uint32_t, uint64_t>::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_i8, ded_u32, ded_i8, typeconv_op< uint32_t, int8_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i16, ded_u32, ded_i16, typeconv_op< uint32_t, int16_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i32, ded_u32, ded_i32, typeconv_op< uint32_t, int32_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i64, ded_u32, ded_i64, typeconv_op< uint32_t, int64_t >::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_flt, ded_u32, ded_flt, typeconv_op< uint32_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_u32, ded_dbl, typeconv_op< uint32_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_u32, ded_bool, typeconv_op< uint32_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}

		// convert from u64
		{
			add_deduction( ded_u8, ded_u64, ded_u8, typeconv_op< uint64_t, int8_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u16, ded_u64, ded_u16, typeconv_op< uint64_t, uint16_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u32, ded_u64, ded_u32, typeconv_op< uint64_t, uint32_t>::create(), code_generator::null() )
				.large_to_small();

			add_deduction( ded_i8, ded_u64, ded_i8, typeconv_op< uint64_t, int8_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i16, ded_u64, ded_i16, typeconv_op< uint64_t, int16_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i32, ded_u64, ded_i32, typeconv_op< uint64_t, int32_t >::create(), code_generator::null() )
				.change_sign();
			add_deduction( ded_i64, ded_u64, ded_i64, typeconv_op< uint64_t, int64_t >::create(), code_generator::null() )
				.change_sign();

			add_deduction( ded_flt, ded_u64, ded_flt, typeconv_op< uint64_t, float >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_dbl, ded_u64, ded_dbl, typeconv_op< uint64_t, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_u64, ded_bool, typeconv_op< uint64_t, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from float
		{
			add_deduction( ded_u8, ded_flt, ded_u8, typeconv_op< float, int8_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u16, ded_flt, ded_u16, typeconv_op< float, uint16_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u32, ded_flt, ded_u32, typeconv_op< float, uint32_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u64, ded_flt, ded_u64, typeconv_op< float, uint64_t>::create(), code_generator::null() )
				.large_to_small();

			add_deduction( ded_i8, ded_flt, ded_i8, typeconv_op< float, int8_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i16, ded_flt, ded_i16, typeconv_op< float, int16_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i32, ded_flt, ded_i32, typeconv_op< float, int32_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i64, ded_flt, ded_i64, typeconv_op< float, int64_t >::create(), code_generator::null() )
				.large_to_small();

			add_deduction( ded_dbl, ded_flt, ded_dbl, typeconv_op< float, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_bool, ded_flt, ded_bool, typeconv_op< float, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from double
		{
			add_deduction( ded_u8, ded_dbl, ded_u8, typeconv_op< double, int8_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u16, ded_dbl, ded_u16, typeconv_op< double, uint16_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u32, ded_dbl, ded_u32, typeconv_op< double, uint32_t>::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_u64, ded_dbl, ded_u64, typeconv_op< double, uint64_t>::create(), code_generator::null() )
				.large_to_small();

			add_deduction( ded_i8, ded_dbl, ded_i8, typeconv_op< double, int8_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i16, ded_dbl, ded_i16, typeconv_op< double, int16_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i32, ded_dbl, ded_i32, typeconv_op< double, int32_t >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_i64, ded_dbl, ded_i64, typeconv_op< double, int64_t >::create(), code_generator::null() )
				.large_to_small();

			add_deduction( ded_flt, ded_dbl, ded_flt, typeconv_op< double, double >::create(), code_generator::null() )
				.large_to_small();
			add_deduction( ded_bool, ded_dbl, ded_bool, typeconv_op< double, bool >::create(), code_generator::null() )
				.large_to_small();
		}
		// convert from bool
		{
			add_deduction( ded_u8, ded_bool, ded_u8, typeconv_op< bool, int8_t>::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_u16, ded_bool, ded_u16, typeconv_op< bool, uint16_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_u32, ded_bool, ded_u32, typeconv_op< bool, uint32_t>::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_u64, ded_bool, ded_u64, typeconv_op< bool, uint64_t>::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_i8, ded_bool, ded_i8, typeconv_op< bool, int8_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_i16, ded_bool, ded_i16, typeconv_op< bool, int16_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_i32, ded_bool, ded_i32, typeconv_op< bool, int32_t >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_i64, ded_bool, ded_i64, typeconv_op< bool, int64_t >::create(), code_generator::null() )
				.small_to_large();

			add_deduction( ded_dbl, ded_bool, ded_dbl, typeconv_op< bool, double >::create(), code_generator::null() )
				.small_to_large();
			add_deduction( ded_flt, ded_bool, ded_flt, typeconv_op< bool, float >::create(), code_generator::null() )
				.small_to_large();
		}
	}
	void add_default_vector_deductions(){
	}
	void add_default_matrix_deductions(){
	}
};