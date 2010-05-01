#ifndef SASL_SYNTAX_TREE_SYMBOL_INFO_H
#define SASL_SYNTAX_TREE_SYMBOL_INFO_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>
#include <string>

BEGIN_NS_SASL_SYNTAX_TREE()

class symbol_info{
public:
	const std::string& class_name(){
		return clsname;
	}
protected:
	symbol_info( std::string& cls ):
		 clsname(cls)
	{}
	std::string clsname;
};

END_NS_SASL_SYNTAX_TREE()

#endif