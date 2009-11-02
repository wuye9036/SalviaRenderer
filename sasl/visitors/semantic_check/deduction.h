/***********************************************************************************
	类型推导依据推导结论分为四级：
		同一、相容、可转换和不可推导。
		同一推导指源类型和目标类型值域相同的推导，等价于is_no_deduction；
		相容推导指目标类型的值域是源类型的超集的推导，等价于is_successful - is_no_deduction；
		可转换推导指源类型于目标类型的值域存在交集的推导，等价于is_maybe_unexcepted - is_successful；
		不可推导指源类型和目标类型值域不相交或语义不一致的推导，等价于is_failed。
		在Deducers框架中，没有显式的注册推导变认为是不可推导的。

	常量-常量推导规则：
		在基本类型的常数类型推导过程中，由于基本类型的值域是可以预先判定的，
		因此仅存在相容推导和不可推导两种情况，相容推导时，不存在警告。不可推导时，需要指定错误。

	常量-变量推导规则：
		从常量类型赋值给变量，会尝试将可以隐性变换的常量进行类型变换后赋值给变量。
		如果不可隐性变换，则会以错误或警告的形式给出显式变换的需求。
		如果隐式变换发生错误，则等同于常量变换的不可推导进行处理。

	变量-变量推导规则：
		变量-变量的推导会给出四级形式的推导结论。
		fail和is_maybe_unexcepted并不会显式的产生error和warning的编译器信息。
		如果转换结果为is_no_deduction和is_successful，则认为转换是可隐式的。
		若转换结果为is_maybe_unexcepted，需要判定转换是隐式转换还是显式转换。
		如果是隐式转换，则会考察can_implicit。如果隐式转换被拒绝，则推导会自动附加相应警告。
		is_failed会始终返回匹配错误。
************************************************************************************/
class deduction{
public:
	static deduction from_scalar_type( buildin_types sasl_type );
	static deduction from_vec_type( buildin_types scalar_type, size_t len );
	static deduction from_mat_type( buildin_types scalar_type, size_t rowcnt, size_t colcnt );

	h_ast_node_type type;
	boost::any value;
	symbol var;

	// if type is same, there is no deduction.
	bool is_no_deduction(){
		return cost_ < match_cost::lossless();
	}
	
	// if deduction executed w/out warning, it is called sucessful
	bool is_sucessful(){
		return cost_ < match_cost::loss();
	}
	
	// if decution maybe leads to unexcepted behaviour, it returns "true"  
	bool is_maybe_unexcepted(){
		return ! is_failed();
	}
	
	// type conversation or value evaluation is failed.
	bool is_failed(){
		return is_err_;
	}
	
	deduction& is_no_deduction( bool v ){
		match_cost()
	}
	
	deduction& is_successful( bool v);
	deduction& is_maybe_unexcepted( bool v );
	deduction& is_failed( bool v );
	deduction& can_implicit( bool v );

	match_cost cost(){
		return cost_;
	}
	
	deduction& cost( match_cost v ){
		cost_ = v;
	}
	
	deduction& add_cost( match_cost v ){
		cost_ += v;
	}

	h_compiler_warning warning(){
		return warning_;
	}
	
	h_compiler_error error(){
		return error_;
	}
	
	deduction& warning( h_compiler_warning v ){
		warning_ = v;
		return *this;
	}
	
	deduction& error( h_compiler_error v ){
		error_ = v;
		return *this;
	}

	//预定义的转换状态设置。
	deduction& small_to_large( size_t cost )		// e.g. int8 -> int16
	{
		cost_ = match_cost( 0, cost, 0 );
	}
	
	deduction& large_to_small( size_t cost )		// e.g. int16 -> int8
	{
		cost_ = match_cost( cost, 0, 0 );
	}
	
	deduction& change_sign( size_t cost )		// e.g. int8 <--> uint8, int8 <--> uint16
	{
		cost_ = match_cost( cost, 0, 0 );
	}

private:
	h_compiler_warning warning_;
	h_compiler_error error_;

	bool can_implicit_;
	bool no_conv_;
	bool is_err_;
	
	match_cost cost_;
};