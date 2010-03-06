class deducer_entry{
	bool operator == ( const deducer_entry& rhs );
	deducer_entry& operator = ( const deducer_entry& rhs );
	dedcuer_entry( const dedcuer_entry& rhs );
	deducer_entry( h_ast_node_type type );
	
	size_t hash_value();
	
	deduction to_deduction();
	
private:
	h_ast_node_type type_;
	size_t hash_code_;
};

struct deducer_value{
	deducer_value(){
	}
	
	bool is_valid(){
		return ded.is_valid();
	}
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
		
		param_match best_match;
		vector< param_match > conflict_matches;
		
		try_match( entries, best_match, conflict_matches, predicator );
		if( conflict_matches.empty() ){
			return best_matches.return_value();
		}
		
		return deducer_value( deduction::ambigous() );
	}
	
private:
	h_typecast_deducer typecast_ded_;
	typedef trie_map< deducer_entry, deducer_value > deducers_map_t;
	deducers_map_t deducers_;
	
	bool is_better_deducer( const deducer_entry& lhs, const deducer_entry& rhs){
		//  如果 T1 和 T2 是相同的类型，则两个转换都不是更好的转换
		if ( lhs.type.is_equal(rhs.type) ){ return 0; }
		
		bool can_lhs2rhs_implicit = typecast_ded_->deduce( lhs, rhs ).can_implicit();
		bool can_rhs2lhs_implicit = typecast_ded_->deduce( rhs, lhs ).can_implicit();
		
		// 如果存在 T1 到 T2 的隐式转换，且不存在 T2 到 T1 的隐式转换，则 C1 是更好的转换
		if ( can_lhs2rhs_implicit && ! can_rhs2lhs_implicit ){
			return 1;
		}
		
		// 如果存在 T2 到 T1 的隐式转换，且不存在 T1 到 T2 的隐式转换，则 C2 是更好的转换
		if ( ! can_lhs2rhs_implicit && can_rhs2lhs_implicit ){
			return -1;
		}
		
		// 如果 T1 为 sbyte 而 T2 为 byte, ushort, int 或者 long，则 C1 是更好的转换
		//	如果 T1 为 sbyte 而 T2 为 byte、ushort、uint 或 ulong，则 C1 为更好的转换。
		// 如果 T2 为 sbyte 而 T1 为 byte、ushort、uint 或 ulong，则 C2 为更好的转换。
		// 如果 T1 为 short 而 T2 为 ushort、uint 或 ulong，则 C1 为更好的转换。
		// 如果 T2 为 short 而 T1 为 ushort、uint 或 ulong，则 C2 为更好的转换。
		// 如果 T1 为 int 而 T2 为 uint 或 ulong，则 C1 为更好的转换。
		// 如果 T2 为 int 而 T1 为 uint 或 ulong，则 C2 为更好的转换。
		// 如果 T1 为 long 而 T2 为 ulong，则 C1 为更好的转换。
		// 如果 T2 为 long 而 T1 为 ulong，则 C2 为更好的转换。
		buildin_types lhs_typecode = lhs.type->get_buildin_typecode();
		buildin_types rhs_typecode = rhs.type->get_buildin_typecode();
		
		if ( is_better_buildin_type_conversation( lhs_typecode, rhs_typecode ) ){
			return 1;
		}
		
		if ( is_better_buildin_type_conversation( rhs_typecode, lhs_typecode ) ){
			return -1;
		}
		
		//否则，C1，C2都不是更好的转换。
		return 0;
	}
	
	bool is_better_buildin_type_conversation(  buildin_types lhs, buildin_types rhs ){
		if ( lhs_typecode == buildin_types::sasl_int8 &&
			(	rhs_typecode == buildin_types::sasl_uint8 ||
				rhs_typecode == buildin_types::sasl_uint16 ||
				rhs_typecode == buildin_types::sasl_uint32 ||
				rhs_typecode == buildin_types::sasl_uint64
			)
		) {
			return true;
		}
		
		if ( lhs_typecode == buildin_types::sasl_int16 &&
			(	rhs_typecode == buildin_types::sasl_uint16 ||
				rhs_typecode == buildin_types::sasl_uint32 ||
				rhs_typecode == buildin_types::sasl_uin64
			)
		) {
			return true;
		}
		
	if ( lhs_typecode == buildin_types::sasl_int32 &&
			(	rhs_typecode == buildin_types::sasl_uint32 ||
				rhs_typecode == buildin_types::sasl_uin64
			)
		) {
			return true;
		}
		
		if ( lhs_typecode == buildin_types::sasl_int64 &&
			(	rhs_typecode == buildin_types::sasl_uin64
			)
		) {
			return true;
		}
		
		return false;
	}
	
	bool deduce_predicator( const deducer_value& v ){
		return ! v.ded.is_failed();
	}
	
	bool evaluate_predicator( const deducer_value&  v ) {
		return v.ct_op != NULL;
	}
	
	bool code_gen_predicator( const deducer_value& v ){
		return v.code_gen != NULL;
	}
	
	bool try_match(
		const vector< deducer_entry >& entries,
		param_match& best_match,
		vector< param_match >& conflict_matches,
		function< bool (const deducer_result&) > predicator
		)
	{
		conflict_matches.clear();
		best_match.initialize();
		
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
				param_match candidate_deduction = match_deduction( entries, trie_map_iter_stack, predicator );
				update_best_matches( candidate_deduction, best_match, conflict_matches );
				++trie_map_iter_stack.back();
				continue;
			}
			
			// 如果当前节点能够匹配entry，那么就再深入一层，否则的话就忽略当前的往后迭代一个。
			size_t iter_idx = trie_map_iter_stack.size() - 1;
			const deducer_entry& cur_entry ( entries[ iter_idx ] );
			const_child_iterator_t& cur_iter( trie_map_iter_stack.back() );
			
			deducer_value cur_value = typecast_ded_.generate_deduce(
				deducer_entry.to_deduction(), 
				(cur_iter->first).to_deduction() 
				);
			
			if( predicator( cur_value ) ){
				deducer_value best_entry = best_match.result_value().params()[iter_idx];
				if ( ! is_better_deducer(best_entry, cur_entry) ){
					trie_map_iter_stack.push_back( cur_iter->child_begin() );	
					continue;
				}
			}
			++cur_iter;
		}
	}
	
	void update_best_matches( 
		const deducer_value& candidate_match, 
		param_match& best_match, 
		vector< param_match >& conflict_matches ){
		if ( candidate_match.is_better_than( best_match ) ){
			conflict_matches.clear();
			best_match = candidate_match;
			return;
		} 
		
		conflict_matches.push_back( candidate_match );
	}
	
	param_match match_deduction( 
		const vector< deducer_entry >& entries,
		const vector< deducers_map_t::const_child_iterator_t >& ref,
		function< bool (const deducer_result&) > predicator
		){
		param_match ret_match;
		
		deducer_value ded_val = ref.back()->value();
		ret_match.result_value( ded_val );
		
		for( size_t i_entry = 0; i_entry < entries.size(); ++i_entry ){
			ret_match.param_deds().push_back(
				typecast_ded_.deduce( entries[i_entry].to_deduction(), ref[i_entry]->first.to_deduction() )
				);
		}
		
		return ret_match;
	}
};