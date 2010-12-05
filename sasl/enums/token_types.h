
#ifndef SASL_ENUMS_TOKEN_TYPES_H
#define SASL_ENUMS_TOKEN_TYPES_H

#include "../enums/enum_base.h" 

struct token_types :
	public enum_base< token_types, uint32_t >
	, public equal_op< token_types >, public value_op< token_types, uint32_t >
{
	friend struct enum_hasher;
private:
	token_types( const storage_type& val, const std::string& name );
	token_types( const storage_type& val ): base_type(val){}
public:
	token_types( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type _comment;
	const static this_type _preprocessor;
	const static this_type _operator;
	const static this_type _whitespace;
	const static this_type _constant;
	const static this_type _newline;
	const static this_type _identifier;
	const static this_type _keyword;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

#endif
