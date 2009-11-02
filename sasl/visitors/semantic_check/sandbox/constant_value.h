// compile time value object. cannot construct in stack or copy for preventing truncating.
class ct_value{
private:
	ct_value& operator = ( const ct_value& );
protected:
	ct_value( h_ast_node_type ast_type ): type( ast_type ){}
	h_ast_node_type type;
	boost::any value;
public:
	virtual void assign( const ct_value& val );
};

#define DEFINE_COMPILE_TIME_TYPE( ct_value_impl, type_node_name, type_node_create_param, cpp_value_type) \
class ct_value_impl { \
public:\
	typedef ct_value_impl this_type; \
	typedef cpp_value_type value_t; \
	value_t value;\
	this_type() { \
		copy_node( type, type_node_name##::create( type_node_create_param ) );\
	} \
	this_type( const value_t& val ): value( val ) { \
		copy_node( type, type_node_name##::create( type_node_create_param ) );\
	} \
	this_type& operator = ( const this_type & rhs ){ \
		type = rhs.type; \
		value = rhs.value; \
		return *this; \
	} \
	void assgin( const ct_value& rhs ){ \
		if( type->is_equivalence( val.type ) ){ \
			value = (( this_type& )rhs).value; \
		} \
		return *this; \
	} \
	this_type( const ct_value& rhs ): type( rhs.type ), value( rhs.value ) { \
	} \
};

DEFINE_COMPILE_TIME_TYPE( ct_int8, ast_node_buildin_type, ( sasl_int8 ), int8_t );
DEFINE_COMPILE_TIME_TYPE( ct_int16, ast_node_buildin_type, ( sasl_int16 ), int16_t );
DEFINE_COMPILE_TIME_TYPE( ct_int32, ast_node_buildin_type, ( sasl_int32 ), int32_t );
DEFINE_COMPILE_TIME_TYPE( ct_int64, ast_node_buildin_type, ( sasl_int64 ), int64_t );

DEFINE_COMPILE_TIME_TYPE( ct_uint8, ast_node_buildin_type, ( sasl_uint8 ), uint8_t );
DEFINE_COMPILE_TIME_TYPE( ct_uint16, ast_node_buildin_type, ( sasl_uint16 ), uint16_t );
DEFINE_COMPILE_TIME_TYPE( ct_uint32, ast_node_buildin_type, ( sasl_uint32 ), uint32_t );
DEFINE_COMPILE_TIME_TYPE( ct_uint64, ast_node_buildin_type, ( sasl_uint64 ), uint64_t );

DEFINE_COMPILE_TIME_TYPE( ct_v2i8, ast_node_vector_type

DECL_HANDLE( ct_convertor );
class ct_convertor{
	virtual ct_value& convert( ct_value& lhs, const ct_value& rhs ) = 0;
	virtual h_ast_node_type source_type();
	virtual h_ast_node_type dest_type();
};

template< typename FromCTValueT, typename ToCTValueT >
class simple_ct_convertor: public ct_convertor{
public:
	static h_ct_convertor create();

	ct_value& convert( ct_type& lhs, const ct_type& rhs ){
		 return (ct_value&)convert( (ToCTValueT& )lhs, (const FromCTValueT&) rhs );
	}
	ToCTValueT& convert( ToCTValueT& lhs, const FromCTValueT& rhs ){
		lhs.value = ( ToCTTypeT::value_t& )rhs;
		return lhs;
	}

	h_ast_node_type source_type(){
		return FromCTValueT().type;
	}

	h_ast_node_type dest_type(){
		return ToCTValueT().type;
	}
};

class value_entry{
	h_ast_node_type type_;
	size_t hash_;
public:
	value_entry( h_ast_node_type type );
	h_ast_node_type get_type();
	size_t hash_value();
	bool operator == ( const value_entry& entry );
};

class constant_value_convertor{
public:
	void add_convertor( h_ct_convertor conv ){
		value_entry src_entry( conv.src_type() );
		if( ! convertors_.has_key(src_entry) ){
			unordered_map[ src_entry ] = unordred_map< value_entry, h_ct_convertor >();
		}
		value_entry dest_entry( conv.dest_type() );
		unordered_map[ src_entry ][ dest_entry ] = conv;
	}

	ct_value& convert( ct_value& lhs, const ct_value& rhs ){
		value_entry lhs_entry( lhs.type );
		value_entry rhs_entry( rhs.type );
	
		//if type is same, it will not converse.
		if( lhs_entry == rhs_entry ){
			lhs.assign( rhs );
			return;
		}

		if( ! convertors_.has_key( lhs_entry ) ){
			return ct_value_null;
		}

		if( ! convertors_[ lhs_entry ].has_key( rhs_entry ) ){
			return convertors_[lhs_entry][rhs_entry]( lhs, rhs );
		}
	}
private:
	void initialize_default_convertors(){
		add_convertor( simple_ct_convertor<ct_int8, ct_int16>::create() );
		add_convertor( simple_ct_convertor<ct_int8, ct_int32>::create() );
		add_convertor( simple_ct_convertor<ct_int8, ct_int64>::create() );
		add_convertor( simple_ct_convertor<ct_int16, ct_int8>::create() );
		add_convertor( simple_ct_convertor<
	}
};