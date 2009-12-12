#ifndef SASL_PARSER_PARSER_FORWARD_H
#define SASL_PARSER_PARSER_FORWARD_H

#define DEFINE_GRAMMAR( GRAMMAR_NAME, SIGNATURE ) \
	template <typename IteratorT, typename LexerT> \
	struct GRAMMAR_NAME : \
		boost::spirit::qi::grammar< IteratorT, SIGNATURE, boost::spirit::qi::in_state_skipper<LexerT> >

#define RULE_DEFINE_HELPER() \
	typedef boost::spirit::qi::in_state_skipper<LexerT> skipper_type; \
	template <typename SignatureT> struct rule { \
		 typedef boost::spirit::qi::rule<IteratorT, SignatureT, skipper_type> type; \
	};
	
#endif //SASL_PARSER_PARSER_FORWARD_H