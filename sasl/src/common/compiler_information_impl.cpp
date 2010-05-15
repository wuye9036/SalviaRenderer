#include <sasl/include/common/compiler_information.h>
#include <cstring>
BEGIN_NS_SASL_COMMON();

compiler_informations compiler_information_impl::id( compiler_informations filter ){
	return info_id & filter;
}

std::string compiler_information_impl::id_str(){
	char idstr[6] = {'\0'};

	idstr[0] = 'U';
	if ( info_id.included( compiler_informations::_compile ) ){
		idstr[0] = 'C';
	} else if ( info_id.included(compiler_informations::_link) ){
		idstr[0] = 'L';
	}

	int idval = (int)( id(compiler_informations::_info_id_mask).to_value() );
	for(size_t idstr_i = 4; idstr_i > 0; --idstr_i){
		int quotient = idval / 10;
		int remainder = idval % 10;
		idstr[idstr_i] = char(remainder + '0');
		idval = quotient;
	}
	return std::string( idstr );
}

compiler_information_impl::compiler_information_impl( compiler_informations infoid )
	: info_id(infoid){
}
END_NS_SASL_COMMON();