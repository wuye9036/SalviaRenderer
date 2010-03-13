#include <sasl/include/syntax_tree/operator_table.h>

using namespace std;

BEGIN_NS_SASL_SYNTAX_TREE()

operator_table::operator_table()
{
	add( "+", operators::add );
	add( "+=", operators::add_assign );
	add( "=", operators::assign );
	add( "&", operators::bit_and );
	add( "&=", operators::bit_and_assign );
	add( "~", operators::bit_not );
	add( "~=", operators::bit_not_assign );
	add( "|", operators::bit_or );
	add( "|=", operators::bit_or_assign );
	add( "^", operators::bit_xor );
	add( "^=", operators::bit_xor_assign );
	add( "/", operators::div );
	add( "/=", operators::div_assign );
	add( "==", operators::equal );
	add( ">", operators::greater );
	add( ">=", operators::greater_equal );
	add( "<<", operators::left_shift );
	add( "<", operators::less );
	add( "<=", operators::less_equal );
	add( "&&", operators::logic_and );
	add( "!", operators::logic_not );
	add( "||", operators::logic_or );
	add( "<<=", operators::lshift_assign );
	add( "!=", operators::not_equal );
	add( "%", operators::mod );
	add( "%=", operators::mod_assign );
	add( "*", operators::mul );
	add( "*=", operators::mul_assign );
	add( "++", operators::prefix_incr );
	add( "--", operators::prefix_decr );
}

operator_table& operator_table::add( const std::string& lit, operators op )
{
	lit2op.insert( std::make_pair( lit, op ) );
	op2lit.insert( std::make_pair( op, lit ) );
}

operator_table& operator_table::instance()
{
	static operator_table op_tbl;
}

operators operator_table::find( const string& lit, bool is_unary /*= false*/, bool is_postfix /*= false*/ ) const
{
	return 
		lit2op[lit] |
		( is_unary ? operators::unary_op : operators::none ) |
		( is_postfix ? operators::postfix_op : operators::none)
		;
}

const string& operator_table::find( operators op ) const
{
	return op2lit[op];
}

END_NS_SASL_SYNTAX_TREE()