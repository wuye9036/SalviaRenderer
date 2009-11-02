class param_match{
public:
	void initialize(){
		result_val_ = deducer_value();
	}
	
	bool is_valid(){
		return result_val_.is_valid();
	}
	
	const std::vector< deduction >& param_deds() const { 
		return param_deductions_;
	}
	
	std::vector< deduction >& param_deds() { 
		return param_deductions_;
	}
	
	const deducer_value& result_value() const{
		return result_val_;
	}
	
	void result_value( const deducer_value& val ){
		result_val_ = val;
	}
	
	bool is_better_than( const param_match& rhs ){
		bool least_one_better = false;
		for( size_t i_param = 0; i_param < size(); ++i_param ){
			if( is_better_deducer( deducer_entry( rhs.param_deductions_[i_param].type ), deducer_entry( param_deductions_[i_param].type ) ) ){
				return false;
			}
			if( is_better_deducer( deducer_entry( param_deductions_[i_param].type ), deducer_entry( rhs.param_deductions_[i_param].type ) ) ){
				least_one_better = true;
			}
		}
		return least_one_better;
	}
private:
	std::vector< deduction > param_deductions_;
	deducer_value result_val_;
}