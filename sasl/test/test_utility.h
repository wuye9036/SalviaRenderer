#ifndef SASL_TEST_TEST_UTILITY_H
#define SASL_TEST_TEST_UTILITY_H

#include <sasl/enums/buildin_type_code.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace sasl{
	namespace common{
		struct token_attr;
	}
	namespace syntax_tree{
		struct buildin_type;
	}
	namespace semantic{
		class symbol;
	}
}

using ::sasl::common::token_attr;
using ::sasl::syntax_tree::buildin_type;
using ::sasl::semantic::symbol;

template <typename SymbolInfoT> void extract_symbol_info( boost::shared_ptr<SymbolInfoT>& syminfo, boost::shared_ptr<symbol> sym );

#endif