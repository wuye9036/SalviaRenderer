class match_cost{
	size_t same_cost_;
	size_t lossless_cost_;
	size_t loss_cost_;
	
public:
	static match_cost loss(){
		return match_cost( 1, 0, 0 );
	}
	
	static match_cost lossless(){
		return match_cost( 0, 1, 0 );
	}
	
	static match_cost same(){
		return match_cost( 0, 0, 1 );
	}
	
	static match_cost infinition(){
		return match_cost(
			std::numeric_limits<size_t>::max,
			std::numeric_limits<size_t>::max,
			std::numeric_limits<size_t>::max,
		);
	}
	
	match_cost( size_t loss_cost, size_t lossless_cost, size_t same_cost ):
		loss_cost_( loss_cost ), lossless_cost_( lossless_cost ), same_cost_( same_cost ){}
		
	match_cost( const match_cost& rhs ): 
		loss_cost_( rhs.loss_cost_ ), lossless_cost_( rhs.lossless_cost_ ), same_cost_( rhs.same_cost_ )
	{}
	
	match_cost& operator = ( const match_cost& rhs ){
		same_cost_ = rhs.same_cost_;
		lossless_cost_ = rhs.lossless_cost_;
		loss_cost_ = rhs.loss_cost_;
	}
	
	bool operator < ( const match_cost& rhs ){
		if ( loss_cost_ < rhs.loss_cost_ ){
			return true;
		}
		if ( loss_cost_ > rhs.loss_cost_ ){
			return false;
		}
		
		if( lossless_cost_ < rhs.lossless_cost_ ){
			return true;
		}
		if ( lossless_cost_ > rhs.lossless_cost_ ){
			return false;
		}
		
		// ignore same_cost_
		// equal
		return false;
	}
	
	bool operator <= ( const match_cost& rhs ){
		return (*this) < rhs || (*this) == rhs;
	}
	
	bool operator == ( const match_cost& rhs ){
		return loss_cost_ == rhs.loss_cost_ &&
			lossless_cost_ == rhs.lossless_cost_;
	}
	
	bool operator >= ( const match_cost&  rhs ){
		return ! ( (*this) < rhs );
	}
	
	bool operator > ( const match_cost& rhs ){
		return ! ( (*this) <= rhs );
	}
	
	bool operator != ( const match_cost& rhs ){
		return !( (*this) == rhs );
	}
};

class param_match_cost{
	std::vector< match_cost > param_convert_costs_;
	deducer_value result_val_;
public:
	const std::vector< match_cost >& param_costs() const { 
		return param_convert_costs_;
	}
	
	std::vector< match_cost >& param_costs() { 
		return param_convert_costs_;
	}
	
	const deducer_value& result_value() const{
		return result_val_;
	}
	
	void result_value( const deducer_value& val ){
		result_val_ = val;
	}
	
	bool is_better_than( const param_match_cost& rhs ){		
		if( result_val_.cost() == match_cost::infinition() ){
			return false;
		}

		if( result_val_.cost() == match_cost::infinition() ){
			return true;
		}
		
		if ( rhs.param_convert_costs_.size() != param_convert_costs_.size() ){
			return false;
		}
		
		bool least_one_better = false;
		bool least_one_worse = false;
		
		for( size_t i_param = 0; i_param < size(); ++i_param ){
			if( param_convert_costs_[i_param] < rhs.param_convert_costs_[i_param] ){
				least_one_better = true;
			}
			
			if( param_convert_cost_[i_param] > rhs.param_convert_costs_[i_param] ){
				least_one_worse = true;
				break;
			}
		}
		
		if( ( ! least_one_worse ) && least_one_better ){
			return true;
		}
		
		return false;
	}
}