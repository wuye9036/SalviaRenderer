class deducer_entry{
	bool operator == ( const deducer_entry& rhs );
	deducer_entry& operator = ( const deducer_entry& rhs );
	dedcuer_entry( const dedcuer_entry& rhs );
	
	size_t hash_value();
	
	deduction to_deduction();
	
private:
	h_ast_node_type type_;
	size_t hash_code_;
};

struct deducer_value{
	deducer_value( const deduction& ded, h_ct_operator ct_op, h_code_generator code_gen )
		: ded( ded ), ct_op( ct_op ), code_gen( code_gen ) 
	{}

	deducer_value& set_params( const vector< deducer_entry >& pars) {
		params_ = pars;
	}
	
	deduction ded;
	h_ct_operator ct_op;
	h_code_generator code_gen;
	vector< deducer_entry > params;
};
	
//保存了一个调用（重载的同名函数、操作符等）的多种类型的重载。
//对于其他操作符，调用deduce/evaluator/gen_code时会自动完成隐式类型转换并匹配。
//对于typecast操作符，不会完成隐式类型匹配工作。
class deducer{
public:
	deducer( h_typecast_deducer typecast_ded): typecast_ded_( typecast_ded ){
	}
	
	deduction& add_deduction( 
		const vector< h_ast_node_type >& src_types, 
		deduction ded,
		const h_ct_operator& op, 
		const h_code_generator& gen 
		)
	{
		vector< deducer_entry > entries( src_types.size() );
		BOOST_FOREACH( src_types, h_ast_node_type src_type ){
			entries.push_back( deducer_entry( src_type ) );
		}
		deducer_value val( deduction, op, gen );
		if( ! deducers_.insert( entries.begin(), entries.end(), val ) ) {
			throw key_exists_exception();
		}
		
		return deducers_.lookup( entries );
	}

	// type conversation check for run time.
	template< typename DeductionIterT >
	deduction deduce( DeductionIterT typelst_begin, DeductionIterT typelst_end ){
		return generic_deduce( typelst_begin, typelst_end ).ded;
	}
	
	// const evaluate for compile time.
	template< typename DeductionIterT >
	deduction evaluate( DeductionIterT varlst_begin, DeductionIterT varlst_end ){
		// get best matched function
		deducer_value ret_val = generic_deduce( typelst_begin, typelst_end );
		
		// convert type of arguments for matching parameter's type
		vector< deduction > converted_arguments( distance( varlst_begin, varlst_end ) );
		vector< deducer_entry >::iterator ref_deducer_entry_iter = ret_val.params.begin();
		
		for( DeductionIterT ded_iter = varlst_begin;
			ded_iter != varlst_end;
			++ded_iter, ++ref_deducer_entry_iter ){
			
			converted_arguments.push_back(
				typecast_ded_->evaluate( *ded_iter, ref_deducer_entry_iter->to_deduction() )
				);
		}
		
		// evaluate
		deduction ret_ded = ret_val.ded;
		ret_ded.value = ret_val.ct_op->evaluate( converted_arguments.begin(), converted_arguments.end() );
		return ret_ded;
	}
	
	// type conversation and op code generation rule.
	typename< typename DeductionIterT >
	deduction gen_code( DeductionIterT varlst_begin, DeductionIterT varlst_end ){
		deducer_value ret_val = generic_deduce( typelst_begin, typelst_end );
		
		// convert type of arguments for matching parameter's type
		vector< deduction > converted_arguments( distance( varlst_begin, varlst_end ) );
		vector< deducer_entry >::iterator ref_deducer_entry_iter = ret_val.params.begin();
		
		for( DeductionIterT ded_iter = varlst_begin;
			ded_iter != varlst_end;
			++ded_iter, ++ref_deducer_entry_iter ){
			
			converted_arguments.push_back(
				typecast_ded_->code_gen( *ded_iter, ref_deducer_entry_iter->to_deduction() )
				);
		}
		
		// evaluate
		deduction ret_ded = ret_val.ded;
		ret_ded.sym = ret_val.code_gen->generate( converted_arguments.begin(), converted_arguments.end() );
		return ret_ded;
	}
	
	template< typename DeductionIterT >
	deducer_value generic_deduce( 
		DeductionIterT varlst_begin, DeductionIterT varlst_end,
		function< bool ( const deducer_value& ) > predicator 
	){
		vector< deducer_entry > entries;
		for( DeductionIterT ded_iter = typelst_begin; ded_iter != typelst_end; ++ded_iter ){
			entries.push_back( deducer_entry ( ded_iter->type ) );
		}
		
		param_match_cost best_match;
		param_match_cost second_best_match;
		
		try_match( entries, best_match, second_best_match );
		if( ! predicator( best_match.return_value() ) ){
			return best_match.return_value();
		}
		if( best_match.is_better_than( second_best_match ) ){
			best_match.return_value().params( entries );
		}
		
		return deducer_value( deduction::ambigous() );
	}
	
private:
	h_typecast_deducer typecast_ded_;
	typedef trie_map< deducer_entry, deducer_value > deducers_map_t;
	deducers_map_t deducers_;
	
	bool deduce_predicator( const deducer_value& v ){
		return ! v.ded.is_failed();
	}
	
	bool evaluate_predicator( const deducer_value&  v ) {
		return v.ct_op != NULL;
	}
	
	bool code_gen_predicator( const deducer_value& v ){
		return v.code_gen != NULL;
	}
	
	void try_match(
		const vector< deducer_entry >& entries,
		param_match_cost& best_match,
		param_match_cost& second_best_match
		function< bool ( const deducer_value& )> predicator
		)
	{
		// DFS 搜索
		vector< deducers_map_t::const_child_iterator_t > trie_map_iter_stack;
		trie_map_iter_stack.push_back( deducers_.begin() );
		
		while( 1 ){
			// 获取top的终止哨兵
			const_child_iterator_t cur_guard_iter = 
				trie_map_iter_stack.size() == 1 ?
				deducers_.child_end() :
				trie_map_iter_stack[ trie_map_iter_stack.size()-2 ]->child_end();
			
			// 如果到了 end 了，需要弹栈
			if( trie_map_iter_stack.back() == cur_guard_iter ){
				trie_map_iter_stack.pop_back();
				if( trie_map_iter_stack.empty() ){
					//如果栈空了，就结束了。
					break;
				} else {
					//否则就在往下递进一次
					++trie_map_iter_stack.back();
					continue;
				}
			}
			
			// 如果正好满了，说明是一种可匹配的情况，则输出。
			if( trie_map_iter_stack.size() == entries.size() ){
				param_match_cost candidate_deduction = match_deduction( entries, trie_map_iter_stack );
				update_best_matches( candidate_deduction, best_match, second_best_match );
				++trie_map_iter_stack.back();
				continue;
			}
			
			// 如果当前节点能够匹配entry，那么就再深入一层，否则的话就忽略当前的往后迭代一个。
			const deducer_entry& cur_entry ( entries[ trie_map_iter_stack.size() - 1 ] );
			const_child_iterator_t& cur_iter( trie_map_iter_stack.back() );
			
			deducer_value cur_value = typecast_ded_.generate_deduce(
					deducer_entry.to_deduction(), 
					(cur_iter->first).to_deduction() 
					);
			
			if( predicator( cur_value ) ){
				++cur_iter;
			} else {
				trie_map_iter_stack.push_back( cur_iter->child_begin() );
			}
		}
	}
	
	void update_best_matches( 
		const deducer_value& candidate_match, 
		param_match_cost& best_match, 
		param_match_cost& second_best_match ){
		
		if ( candidate_match.is_better_than( best_match ) ){
			second_best_match = best_match;
			best_match = candidate_match;
			return;
		} 
		
		if ( candidate_match.is_better_than( second_best_match ) ){
			second_best_match = candidate_match;
		}
	}
	
	param_match_cost match_params( 
		const vector< deducer_entry >& entries,
		const vector< deducers_map_t::const_child_iterator_t >& ref,
		function< bool (const deducer_result&) > predicator
		){
		deducer_value costed_deducer_value = ref.back()->value();
		param_match_cost match_cost.result_value( costed_deducer_value );
		
		if( predicator( costed_deducer_value ) ){
			return param_match_cost();
		}
		
		for( size_t i_entry = 0; i_entry < entries.size(); ++i_entry ){
			match_cost.param_costs().push_back(
				typecast_ded_.deduce( entries[i_entry].to_deduction(), ref[i_entry]->first.to_deduction() ).cost()
				);
		}
		
		return match_cost;
	}
};