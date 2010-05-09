#include <sasl/include/semantic/symbol_infos.h>
#include <sasl/include/syntax_tree/constant.h>
#include <sasl/include/syntax_tree/declaration.h>

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::constant;

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

type_symbol_info::type_symbol_info( type_types ttype )
	: base_type( "ref symbol info" ), ttype(ttype) { }
type_symbol_info::type_symbol_info(): base_type( "ref symbol info" ), ttype(type_types::none) { }

boost::shared_ptr<node> type_symbol_info::full_type() const{
	return type_node.lock();
}
void type_symbol_info::full_type( boost::shared_ptr<node> ftnode ){
	type_node = ftnode;
}
type_types type_symbol_info::type_type() const{
	return ttype;
}

variable_symbol_info::variable_symbol_info( bool is_local )
	: isloc( is_local ), base_type( "variable symbol info" )
{
}
variable_symbol_info::variable_symbol_info()
	: isloc(false), base_type( "variable symbol info" )
{
}
bool variable_symbol_info::is_local() const{
	return isloc;
}

END_NS_SASL_SEMANTIC();