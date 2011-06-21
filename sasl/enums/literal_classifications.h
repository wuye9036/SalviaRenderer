
#ifndef SASL_ENUMS_LITERAL_CLASSIFICATIONS_H
#define SASL_ENUMS_LITERAL_CLASSIFICATIONS_H

#include "../enums/enum_base.h" 

struct literal_classifications :
	public enum_base< literal_classifications, uint32_t >
	, public equal_op< literal_classifications >
{
	friend struct enum_hasher;
private:
	literal_classifications( const storage_type& val, const std::string& name );
	literal_classifications( const storage_type& val ): base_type(val){}
public:
	static void force_initialize();
	
	literal_classifications( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type real;
	const static this_type none;
	const static this_type string;
	const static this_type character;
	const static this_type boolean;
	const static this_type integer;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

#endif
