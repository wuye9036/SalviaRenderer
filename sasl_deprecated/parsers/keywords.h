#ifndef SASL_KEYWORDS_H
#define SASL_KEYWORDS_H

namespace keywords{
	const template<typename ScannerT> rule<ScannerT> if_p = str_p("if");
	const template<typename ScannerT> rule<ScannerT> else_p = str_p("else");
}

#endif