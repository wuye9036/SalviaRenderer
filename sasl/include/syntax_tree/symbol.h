#ifndef SASL_SYNTAX_TREE_SYMBOL_H
#define SASL_SYNTAX_TREE_SYMBOL_H

#include "syntax_tree_fwd.h"
#include <boost/smart_ptr.h>
#include <boost/tr1/unordered_map.hpp>
#include <string>

BEGIN_NS_SASL_SYNTAX_TREE()

struct node;
struct symbol_info;

class symbol{
public:
	symbol(boost::shared_ptr<symbol> parent, boost::shared_ptr<node> correspond_node);

	boost::shared_ptr<symbol> find_this(const std::string& s);
	boost::shared_ptr<symbol> find_all(const std::string& s);
	boost::shared_ptr<symbol> add_child(const std::string& s, boost::shared_ptr<node> pnode);

	boost::shared_ptr<symbol_info> symbol_info();
	boost::shared_ptr<node> node();

private:
	typedef std::tr1::unordered_map< std::string, boost::shared_ptr<symbol> > children_t;
	typedef typename children_t::iterator children_iterator_t;

	boost::weak_ptr<node> correspond_node;
	boost::weak_ptr<node> parent;
	children_t children;
	boost::shared_ptr< symbol_info > sym_info;
}

END_NS_SASL_SYNTAX_TREE(()


#endif