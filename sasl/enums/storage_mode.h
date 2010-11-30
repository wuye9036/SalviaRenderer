
#ifndef SASL_ENUMS_STORAGE_MODE_H
#define SASL_ENUMS_STORAGE_MODE_H

#include "../enums/enum_base.h" 

struct storage_mode :
	public enum_base< storage_mode, uint32_t >
	, public equal_op< storage_mode >, public value_op< storage_mode, uint32_t >
{
	friend struct enum_hasher;
private:
	storage_mode( const storage_type& val ): base_type( val ){}
	
public:
	storage_mode( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type none;
	const static this_type constant;
	const static this_type stack_based_address;
	const static this_type register_id;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

const storage_mode storage_mode::none ( UINT32_C( 0 ) );
const storage_mode storage_mode::constant ( UINT32_C( 1 ) );
const storage_mode storage_mode::stack_based_address ( UINT32_C( 3 ) );
const storage_mode storage_mode::register_id ( UINT32_C( 2 ) );


#endif
