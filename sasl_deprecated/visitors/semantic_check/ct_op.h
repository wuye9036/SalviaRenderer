DECL_HANDLE( ct_op );
class ct_op{
	virtual deduction evaluate( const deduction& ref_deduction, const vector< deduction >& args ) = 0;
};