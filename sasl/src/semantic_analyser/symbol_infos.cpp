#include <sasl/include/semantic_analyser/symbol_infos.h>
#include <sasl/include/syntax_tree/constant.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/symbol_info.h>

BEGIN_NS_SASL_SEMANTIC_ANALYSER();

using sasl::syntax_tree::constant;

value_symbol_info::value_symbol_info(): value_symbol_info::base_type( "value symbol info" ){}
value_symbol_info::value_symbol_info( const constant& c ): value_symbol_info::base_type( "value symbol info" ){
	if (c.valtype == literal_constant_types::integer ){
		if ( c.is_unsigned() ){
			val = boost::lexical_cast<unsigned long>(c.littok.lit);
		} else {
			val = boost::lexical_cast<long>(c.littok.lit);
		}
	} else if( c.valtype == literal_constant_types::real ){
		val = boost::lexical_cast<double>(c.littok.lit);
	} else if( c.valtype == literal_constant_types::boolean ){
		val = (c.littok.lit == "true");
	} else if( c.valtype == literal_constant_types::character ){
		val = c.littok.lit[0];
	} else if( c.valtype == literal_constant_types::string ){
		val = c.littok.lit;
	}
}

value_type_symbol_info::value_type_symbol_info(): value_type_symbol_info::base_type( "value symbol info" ){}
value_type_symbol_info::value_type_symbol_info( boost::shared_ptr<type_specifier> spec )
	: value_type_symbol_info::base_type( "value symbol info" ), valtype(spec)
{}

boost::shared_ptr<type_specifier> value_type_symbol_info::value_type(){
	return valtype;
}

END_NS_SASL_SEMANTIC_ANALYSER();