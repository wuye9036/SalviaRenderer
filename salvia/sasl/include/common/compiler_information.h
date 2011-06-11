#ifndef SASL_COMMON_COMPILER_INFORMATION_H
#define SASL_COMMON_COMPILER_INFORMATION_H

#include <sasl/include/common/common_fwd.h>
#include <sasl/enums/compiler_informations.h>
#include <string>

BEGIN_NS_SASL_COMMON();

class compiler_information{
public:
	virtual compiler_informations id( compiler_informations filter ) = 0;
	virtual std::string id_str() = 0;
	virtual std::string desc() = 0;
protected:
	compiler_information(){}
	~compiler_information(){}
};

class compiler_information_impl: public compiler_information{
public:
	virtual compiler_informations id( compiler_informations filter );
	virtual std::string id_str();
	virtual std::string desc() = 0;
protected:
	compiler_information_impl( compiler_informations infoid );
	compiler_informations info_id;
private:
	compiler_information_impl( const compiler_information_impl& );
	compiler_information_impl& operator = ( const compiler_information_impl& );
};

END_NS_SASL_COMMON();

#endif