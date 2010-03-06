struct ct_unary_op: public ct_op{
	deduction evaluate( const deduction& ref_deduction, const deduction& lhs );
	
	deduction evaluate( const deduction& ref_deduction, const vector< deduction >& args ){
		evaluate( ref_deduction, arguments[0] );
	}
};

#define DEFINE_CT_UNARY_OP ( name, op ) \
	template< typename ResultT, typename ParamT > \
	struct name: public ct_op { \
		deduction evaluate( const deduction& ref_deduction, const deduction& lhs ){ \
			ParamT lhs_val = any_cast<ParamT>( lhs_val.value ); \
			return (ResultT)( op lhs ); \
		} \
	};

DEFINE_CT_UNARY_OP( ct_op_pos, + );
DEFINE_CT_UNARY_OP( ct_op_neg, - );
DEFINE_CT_UNARY_OP( ct_op_bit_neg, ~ );
DEFINE_CT_UNARY_OP( ct_op_not, ! );

// ++ -- 操作符不可应用到常数上。
struct ct_op_prefix_incr{
	deduction evaluate( const deduction& ref_deduction, const deduction& lhs ){
		// error
	}
};

struct ct_op_prefix_decr{
	deduction evaluate( const deduction& ref_deduction, const deduction& lhs ){
		// error
	}
};

struct ct_op_postfix_incr{
	deduction evaluate( const deduction& ref_deduction, const deduction& lhs ){
		// error
	}
};

struct ct_op_postfix_decr{
	deduction evaluate( const deduction& ref_deduction, const deduction& lhs ){
		// error
	}
};