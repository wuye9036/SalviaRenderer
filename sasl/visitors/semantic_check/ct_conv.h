DECL_HANDLE( ct_value_convertor );
class ct_value_convertor{
public:
	virtual deduction convert( const deduction& ref_deduction, boost::any v ) = 0;
};

namespace detail{
	// 类型序列
	typedef vector< int8_t, int16_t, int32_t, int64_t > signed_ints;
	typedef vector< uint8_t, uint16_t, uint32_t, uint64_t > unsigned_ints;
	typedef vector< bool, int8_t, int16_t, int32_t, int64_t, float, double > signed_scalars;
	typedef vector< bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double > unsigned_scalars;

	// 类型对，从小到大升序排列
	template< typename FirstT = void, typename SecondT = void >
	struct type_pair{
		typedef FirstT first_t;
		typedef SecondT second_t;
	};

	template< typename TypeVectorT >
	struct make_type_pairs_from_vector{
		template< typename FirstT, typename SecondTBegin, typename SecondTEnd >
		struct make_first_binded_pairs{
			typedef 
				if_< is_same< SecondTBegin, SecondTEnd >,
				vector<>,
				push_front<
				type_pair<FirstT, deref<SecondTBegin>::type>, 
				make_first_binded_pairs< FirstT, next<SecondTBegin>::type, SecondTEnd
				>::type
				>::type type;
		};

		template< typename FirstTBegin, typename FirstTEnd >
		struct make_pairs{
			typedef 
				if_< is_same< FirstTBegin, FirstTEnd >,
				vector<>,
				insert_range<
				make_first_binded_pairs< deref<FirstTBegin>::type, next<FirstTBegin>::type, FirstTEnd >
				>::type
				>::type type;
		};
		typedef make_pairs< begin<TypeVectorT>::type, end<TypeVectorT>::type >::type type;
	};

	template< typename TypeVectorT0, typename TypeVectorT1 >
	struct make_type_pairs_from_two_vectors{
		template< typename FirstT, typename SecondTBegin, typename SecondTEnd >
		struct first_binded_pairs {
			typedef if_< 
				is_same< SecondTBegin, SecondTEnd >, 
				vector<>, first_binded_pairs< FirstT, next< SecondTBegin >::type, SecondTEnd >::type_pairs
			>::type sub_pairs;
			typedef push_front< sub_pairs, type_pair< FirstT, deref<SecondTBegin>::type > >::type type_pairs;
		};

		template< typename FirstTBegin, typename FirstTEnd, typename SecondVectorT >
		struct pair_list {
			typedef if_<
				is_same< FirstTBegin, FirstTEnd >,
				vector<>, pair_list< next< FirstTBegin >::type, FirstTEnd, SecondVectorT >::type_pairs
			>::type sub_pairs;
			typedef insert_range<
				sub_pairs,
				begin<sub_pairs>::type,
				first_binded_pairs<
				deref<FirstTBegin>,
				begin<SecondVectorT>::type, 
				end<SecondVectorT>::type 
				>::type_pairs
			>::type type_pairs;
		};

		typedef pair_list< begin<TypeVectorT0>::type, end<TypeVectorT0>::type, TypeVectorT1 >::type_pairs type;
	};
	
	typedef make_type_pairs_from_vector< signed_scalars >::type signed_type_pairs;
	typedef make_type_pairs_from_vector< unsigned_scalars >::type unsigned_type_pairs;
	typedef make_type_pairs_from_two_vectors< singed_ints, unsigned_ints > sign_to_unsign_type_pairs;

	// 类型转换traits
	template< typename PairT0, typename PairT1 >
	struct is_same_pair
		: public and_<
		is_same< PairT0::first_t, PairT1::first_t >,
		is_same< PairT1::second_t, PairT1::second_t >
		>
	{};

	template< typename PairVectorT, typename PairT >
	struct is_contains_pair
		: public 
		not_< is_same<
		find_if< PairVectorT, is_same_pair< _1, PairT > >::type,
		end< PairVectorT >::type
		> >
	{};

	template< typename PairT >
	struct is_small_to_large
		: public or_< 
			is_contains_pair< unsinged_type_pairs, PairT >,
			is_contains_pair< signed_type_pairs, PairT > 
			>
	{};

	template< typename PairT >
	struct is_large_to_small
		: public is_small_to_large< type_pair< PairT::second_t, PairT::first_t > >
	{};

	template< typename PairT >
	struct is_sign_to_unsign
		: public is_contains_pair< sign_to_unsign_type_pairs, PairT >
	{};

	template< typename PairT >
	struct is_unsign_to_sign
		: public is_contains_pair< sign_to_unsign_type_pairs,type_pair< PairT::second_t, PairT::first_t > >
	{};
}

template< typename SourceT, typename DestT >
class ct_buildin_value_convertor: public ct_value_convertor{
	static h_ct_value_convertor create(){
		return h_ct_value_convertor( new ct_buildin_value_convertor<SourceT, DestT>() );
	}

	deduction convert( const deduction& ref_deduction, boost::any v ){
		return convert_impl( ref_deduction, v );
	}

private:
	// 常量类型转换分为以下几种类型：
	//	低到高
	//	高到低
	//	有符号-无符号
	//	无符号-有符号
	// 运用 enable_if 四选一
	deduction convert_impl( const deduction& ref_deduction, boost::any v, enable_if< is_small_to_large<SourceT, DestT> >::type* dummy = 0){
		deduction ret = ref_deduction;
		ret.value = boost::any( (DestT)( any_cast<SourceT>(v) ) );
		return ret;
	}

	deduction convert_impl( const deduction& ref_deduction, boost::any v, enable_if< is_large_to_small<SourceT, DestT> >::type* dummy = 0){
		SourceT ret_val = any_cast<SourceT>( v );
		if( (DestT)ret_val > numeric_limits< SourceT >::max ){
			deduction err_deduction = ref_deduction;
			// TODO: add constant conversation error.
			return err_deduction;
		}

		deduction ret_ded = ref_deduction;
		ret_ded.value = boost::any( (DestT)( any_cast<SourceT>(v) ) );
		return ret_ded;
	}

	deduction convert_impl( const deduction& ref_deduction, boost::any v, enable_if< is_sign_to_unsign<SourceT, DestT> >::type* dummy = 0){
		SourceT src_val = any_cast< SourceT >( v );
		if( src_val < 0 ){
			// TODO: error
		}
		if ( (uint64_t)src_val > (uint64_t)numeric_limits<DestT>::max ) {
			// TODO: error
		}

		deduction ret_ded = ref_deduction;
		ret_ded.value = boost::any( (DestT)( any_cast<SourceT>(v) ) );
		return ret_ded;
	}

	deduction convert_impl( const deduction& ref_deduction, boost::any v, enable_if< is_unsign_to_sign<SourceT, DestT> >::type* dummy = 0){
		SourceT src_val = any_cast< SourceT >( v );
		uint64_t sign_upper_bound = numeric_limits<DestT>::max / 2;
		if( (uint64_t)src_val > ( uint64_t )sign_upper_bound ){
			// TODO: error
		}

		deduction ret_ded = ref_deduction;
		ret_ded.value = boost::any( (DestT)( any_cast<SourceT>(v) ) );
		return ret_ded;
	}
};

