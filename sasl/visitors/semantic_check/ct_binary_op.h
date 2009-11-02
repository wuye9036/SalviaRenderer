struct ct_binop: public ct_op{
	deduction evaluate( const deduction& ref_deduction, const vector< deduction >& args ){
		return evaluate( ref_deduction, args[0], args[1] );
	}
	
	virtual deduction evaluate( const deduction& ref_deduction, const deduction& lhs, const deduction& rhs ) = 0;
};

#define DEFINE_CT_BINOP ( name, op ) \
template< typename ResultValueT, typename LValueT, typename RValueT > \
struct name :public ct_binop { \
	deduction evaluate( const deduction& ref_deduction, const deduction& lhs, const deduction& rhs ){ \
		LValueT lhs_val = any_cast<LVauleT>( lhs.value ); \
		RValueT rhs_val = any_cast<RValueT>( rhs.value ); \
		deduction ret_deduction = ref_deduction; \
		ret_deduction.value = any( (ResultT)( lhs_val op rhs_val ) ) ; \
		return ret_deduction; \
	} \
};

DEFINE_CT_BINOP( ct_op_add, + );
DEFNIE_CT_BINOP( ct_op_sub, - );
DEFINE_CT_BINOP( ct_op_mul, * );
DEFINE_CT_BINOP( ct_op_div, / );

DEFINE_CT_BINOP( ct_op_ne, != );
DEFINE_CT_BINOP( ct_op_lt, < );
DEFINE_CT_BINOP( ct_op_le, <= );
DEFINE_CT_BINOP( ct_op_eq, == );
DEFINE_CT_BINOP( ct_op_ge, >= );
DEFINE_CT_BINOP( ct_op_gt, > );

DEFINE_CT_BINOP( ct_op_and, && );
DEFINE_CT_BINOP( ct_op_or, || );

DEFINE_CT_BINOP( ct_op_bit_and, & );
DEFINE_CT_BINOP( ct_op_bit_or, | );
DEFINE_CT_BINOP( ct_op_bit_xor, ^ );

DEFINE_CT_BINOP( ct_op_ls, << );
DEFINE_CT_BINOP( ct_op_rs, >> );
