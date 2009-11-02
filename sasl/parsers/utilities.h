#ifndef SASL_PARSER_UTILITIES_H
#define SASL_PARSER_UTILITIES_H

#define GRAMMAR_BEGIN( grammar_name ) \
struct grammar_name: grammar< grammar_name > { \
	grammar_name();\
	template< typename ScannerT> struct definition{\
	definition(const grammar_name& self);

#define GRAMMAR_END( start_rule_name )\
		const RULE_TYPE( start_rule_name )& start() const;\
	}; /*of definition*/\
};

#define DEFINE_GRAMMAR_DEFINITION( grammar_name ) \
	template< typename ScannerT > grammar_name::definition< ScannerT >::definition(const grammar_name& self)

#define DEFINE_START_RULE( grammar_name, start_rule_name ) \
	template< class ScannerT>\
	const RULE_TYPE( start_rule_name )& grammar_name::definition<ScannerT>::start() const{ return start_rule_name; }

#endif