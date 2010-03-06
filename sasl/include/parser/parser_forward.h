#ifndef SASL_PARSER_PARSER_FORWARD_H
#define SASL_PARSER_PARSER_FORWARD_H

#define SASL_GRAMMAR_ATTRIBUTE_ENABLED

#if defined(SASL_GRAMMAR_ATTRIBUTE_ENABLED)

#define SASL_DEFINE_GRAMMAR( GRAMMAR_NAME, SIGNATURE ) \
	template <typename IteratorT, typename LexerT> \
	struct GRAMMAR_NAME : \
		boost::spirit::qi::grammar< IteratorT, SIGNATURE, boost::spirit::qi::in_state_skipper<LexerT> >

#define SASL_GRAMMAR_RULE_DEFINITION_HELPER() \
	typedef boost::spirit::qi::in_state_skipper<LexerT> skipper_type; \
	template <typename SignatureT> struct rule { \
		 typedef boost::spirit::qi::rule<IteratorT, SignatureT, skipper_type> type; \
	};

#else

#define SASL_DEFINE_GRAMMAR( GRAMMAR_NAME, SIGNATURE ) \
	template <typename IteratorT, typename LexerT> \
	struct GRAMMAR_NAME : \
	boost::spirit::qi::grammar< IteratorT, boost::spirit::qi::in_state_skipper<LexerT> >

#define SASL_GRAMMAR_RULE_DEFINITION_HELPER() \
	typedef boost::spirit::qi::in_state_skipper<LexerT> skipper_type; \
	template <typename SignatureT> struct rule { \
		 typedef boost::spirit::qi::rule<IteratorT, skipper_type> type; \
	};

#endif // defined(SASL_GRAMMAR_ATTRIBUTE_ENABLED)

template <typename IteratorT, typename LexerT, typename TokenDefT> class sasl_grammar;

#endif //SASL_PARSER_PARSER_FORWARD_H