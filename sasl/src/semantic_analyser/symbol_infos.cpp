#include <sasl/include/semantic_analyser/symbol_infos.h>
#include <sasl/include/syntax_tree/constant.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/symbol_info.h>

BEGIN_NS_SASL_SEMANTIC_ANALYSER();

value_symbol_info::value_symbol_info(): base_type( "value symbol info" ){}
value_symbol_info::value_symbol_info( const constant& c ): base_type( "value symbol info" ){
	if (c.valtype == literal_constant_types::integer ){
		if ( c.is_unsigned() ){
			val = boost::lexical_cast<unsigned long>(c.lit);
		} else {
			val = boost::lexical_cast<long>(c.lit);
		}
	} else if( c.valtype == literal_constant_types::real ){
		val = boost::lexical_cast<double>(c.lit);
	} else if( c.valtype == literal_constant_types::boolean ){
		val = (c.lit == "true");
	} else if( c.valtype == literal_constant_types::character ){
		val = c.lit[0];
	} else if( c.valtype == literal_constant_types::string ){
		val = c.lit;
	}
}

value_type_symbol_info::value_type_symbol_info(): base_type( "value symbol info" ){}
value_type_symbol_info::value_type_symbol_info( boost::shared_ptr<type_specifier> spec )
	: base_type( "value symbol info" ), spec(spec)
{}

boost::shared_ptr<type_specifier> value_type_symbol_info::value_type(){
	return valtype;
}

END_NS_SASL_SEMANTIC_ANALYSER();