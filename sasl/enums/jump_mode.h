
#ifndef SASL_ENUMS_JUMP_MODE_H
#define SASL_ENUMS_JUMP_MODE_H

#include "../enums/enum_base.h" 

struct jump_mode :
	public enum_base< jump_mode, uint32_t >
	, public equal_op< jump_mode >
{
	friend struct enum_hasher;
private:
	jump_mode( const storage_type& val ): base_type( val ){}
	
public:
	jump_mode( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type _return;
	const static this_type none;
	const static this_type _continue;
	const static this_type _break;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

#endif
