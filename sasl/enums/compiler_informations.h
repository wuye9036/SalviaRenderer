
#ifndef SASL_ENUMS_COMPILER_INFORMATIONS_H
#define SASL_ENUMS_COMPILER_INFORMATIONS_H

#include "../enums/enum_base.h" 

struct compiler_informations :
	public enum_base< compiler_informations, uint32_t >
	, public bitwise_op< compiler_informations >, public equal_op< compiler_informations >, public value_op< compiler_informations, uint32_t >
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
	std::string name() const;

};

const compiler_informations compiler_informations::none ( UINT32_C( 0 ) );
const compiler_informations compiler_informations::_error ( UINT32_C( 131072 ) );
const compiler_informations compiler_informations::_message ( UINT32_C( 262144 ) );
const compiler_informations compiler_informations::redef_diff_basic_type ( UINT32_C( 16909290 ) );
const compiler_informations compiler_informations::_link ( UINT32_C( 33554432 ) );
const compiler_informations compiler_informations::_info_level_mask ( UINT32_C( 16711680 ) );
const compiler_informations compiler_informations::_stage_mask ( UINT32_C( 4278190080 ) );
const compiler_informations compiler_informations::unknown_compile_error ( UINT32_C( 16909289 ) );
const compiler_informations compiler_informations::uses_a_undef_type ( UINT32_C( 16909292 ) );
const compiler_informations compiler_informations::_info_id_mask ( UINT32_C( 65535 ) );
const compiler_informations compiler_informations::_compile ( UINT32_C( 16777216 ) );
const compiler_informations compiler_informations::redef_cannot_overloaded ( UINT32_C( 16909291 ) );
const compiler_informations compiler_informations::_warning ( UINT32_C( 65536 ) );


#endif
