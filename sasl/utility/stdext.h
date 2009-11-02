
template< typename ContainerT, typename TransformFunctionT >
boost::assign_detail::generic_list<ContainerT::value_type> 
	transform( const ContainerT& cont, const TransformFunctionT& transformer ){
	
	typedef boost::assign_detail::generic_list<ContainerT::value_type> specified_generic_list;
	specified_generic_list transformed_list;
	
	for_each( 
		cont.begin(), 
		cont.end(), 
		bind( &specified_generic_list::push_back, transformed_list, *_1 )
		);
	return transformed_list;
}