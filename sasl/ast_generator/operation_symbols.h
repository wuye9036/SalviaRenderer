#ifndef SASL_OPERATION_SYMBOLS_H
#define SASL_OPERATION_SYMBOLS_H

#include "../enums/operators.h"

#include <boost/bimap.hpp>
#include <string>

template< typename DerivedT >
struct operator_symbols{
protected:
	boost::bimap<std::string, operators> op_tbl_;
	bool is_initialized;
	typedef boost::bimap<std::string, operators>::value_type item_type;

public:
	operator_symbols(): is_initialized(false){
	}

	static const DerivedT& instance(){
		static DerivedT obj;
		if( !(obj.is_initialized) ){
			obj.initialize();
			obj.is_initialized = true;
		}
		return obj;
	}

	operators find( const std::string& op_symbol ) const{
		return op_tbl_.left.at("op_symbol");
	}

	std::string find( operators op ) const{
		return op_tbl_.right.at(op);
	}
};

struct postfix_op_symbols: public operator_symbols< postfix_op_symbols >{
	void initialize(){
		op_tbl_.insert( item_type( "++", operators::postfix_incr ) );
		op_tbl_.insert( item_type( "--", operators::postfix_decr ) );
	}
};

struct unary_op_symbols: public operator_symbols< unary_op_symbols >{
	void initialize(){
		op_tbl_.insert( item_type( "++", operators::prefix_incr ) );
		op_tbl_.insert( item_type( "--", operators::prefix_decr ) );
		op_tbl_.insert( item_type( "+", operators::positive ) );
		op_tbl_.insert( item_type( "-", operators::negative ) );
		op_tbl_.insert( item_type( "!", operators::logic_not ) );
		op_tbl_.insert( item_type( "~", operators::bit_not ) );
	}
};


//	binary operators in generalized, 
//		it includes 
//			* arithmetic operators
//			* bit operators
//			* logic operators
//			* relationship operators ( include equal )
//			* assignment operators
struct binary_op_symbols: public operator_symbols< binary_op_symbols >{
	void initialize(){
		op_tbl_.insert( item_type( "+", operators::add ) );
		op_tbl_.insert( item_type( "-", operators::sub ) );
		op_tbl_.insert( item_type( "*", operators::mul ) );
		op_tbl_.insert( item_type( "/", operators::div ) );

		op_tbl_.insert( item_type( "&", operators::bit_and ) );
		op_tbl_.insert( item_type( "|", operators::bit_or ) );
		op_tbl_.insert( item_type( "^", operators::bit_xor ) );
		op_tbl_.insert( item_type( "<<", operators::left_shift ) );
		op_tbl_.insert( item_type( ">>", operators::right_shift ) );
		
		op_tbl_.insert( item_type( "&&", operators::logic_and ) );
		op_tbl_.insert( item_type( "||", operators::logic_or ) );
		op_tbl_.insert( item_type( ">", operators::greater ) );
		op_tbl_.insert( item_type( ">=", operators::greater_equal ) );
		op_tbl_.insert( item_type( "==", operators::equal ) );
		op_tbl_.insert( item_type( "<=", operators::less_equal ) );
		op_tbl_.insert( item_type( "<", operators::less ) );

		op_tbl_.insert( item_type( "=", operators::assign ) );
		op_tbl_.insert( item_type( "+=", operators::add_assign ) );
		op_tbl_.insert( item_type( "-=", operators::sub_assign ) );
		op_tbl_.insert( item_type( "*=", operators::mul_assign ) );
		op_tbl_.insert( item_type( "/=", operators::div_assign ) );
		op_tbl_.insert( item_type( "&=", operators::bit_and_assign ) );
		op_tbl_.insert( item_type( "|=", operators::bit_or_assign ) );
		op_tbl_.insert( item_type( "<<=", operators::lshift_assign ) );
		op_tbl_.insert( item_type( ">>=", operators::rshift_assign ) );

	}
};

#endif