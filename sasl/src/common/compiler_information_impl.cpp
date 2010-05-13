#include <sasl/include/common/compiler_information.h>
#include <cstring>
BEGIN_NS_SASL_COMMON();

compiler_informations compiler_information_impl::id( compiler_informations filter ){
	return info_id & filter;
}

std::string compiler_information_impl::id_str(){
	char idstr[7] = {'\0'};

	idstr[0] = 'U';
	if( info_id.included( compiler_informations::_warning ) ){
		idstr[0] = 'W';
	} else if ( info_id.included( compiler_informations::_error ) ){
		idstr[0] = 'E';
	} else if ( info_id.included( compiler_informations::_message ) ){
		idstr[0] = 'M';
	}

	idstr[1] = 'U';
	if ( info_id.included( compiler_informations::_compile ) ){
		idstr[1] = 'C';
	} else if ( info_id.included(compiler_informations::_link) ){
		idstr[1] = 'L';
	}

	int idval = (int)( id(compiler_informations::_info_id_mask).to_value() );
	for(size_t idstr_i = 6; idstr_i > 2; --idstr_i){
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