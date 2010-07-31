
#ifndef SASL_ENUMS_COMPILER_INFORMATIONS_H
#define SASL_ENUMS_COMPILER_INFORMATIONS_H

#include "../enums/enum_base.h" 

struct compiler_informations :
	public enum_base< compiler_informations, int >
	, public bitwise_op< compiler_informations >, public equal_op< compiler_informations >, public value_op< compiler_informations, int >
{
	friend struct enum_hasher;
private:
	compiler_informations( const storage_type& val ): base_type( val ){}
	
public:
	compiler_informations( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type none;
	const static this_type _error;
	const static this_type _message;
	const static this_type redef_diff_basic_type;
	const static this_type _link;
	const static this_type _info_level_mask;
	const static this_type _stage_mask;
	const static this_type unknown_compile_error;
	const static this_type uses_a_undef_type;
	const static this_type _info_id_mask;
	const static this_type _compile;
	const static this_type redef_cannot_overloaded;
	const static this_type _warning;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );

};
#endif
