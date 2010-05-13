#include <sasl/include/semantic/semantic_error.h>

BEGIN_NS_SASL_SEMANTIC_ERRORS();

boost::shared_ptr<semantic_error> semantic_error::create( compiler_informations info ){
	return boost::shared_ptr<semantic_error>( new semantic_error( info ) );
}

semantic_error::semantic_error( compiler_informations info )
	: compiler_information_impl( info ){}

std::string semantic_error::desc(){
	return id_str() + std::string(":") + compiler_informations::to_name( info_id ) ;
}
END_NS_SASL_SEMANTIC_ERRORS();