#include <sasl/include/semantic/semantics.h>

#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/pety.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/common/diag_chat.h>
#include <salviar/include/shader.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/pool/pool.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

EFLIB_USING_SHARED_PTR(sasl::syntax_tree, program);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);
EFLIB_USING_SHARED_PTR(sasl::common, diag_chat);
using boost::unordered_map;
using std::vector;
using std::string;
using std::make_pair;

BEGIN_NS_SASL_SEMANTIC();

class module_semantic_impl: public module_semantic
{
public:
	module_semantic_impl(): node_semantic_pool_( sizeof(node_semantic) )
	{
		pety_ = pety_t::create();
		root_symbol_ = symbol::create_root();
		diag_chat_ = diag_chat::create();
		pety_->root_symbol( root_symbol_.get() );
	}

	~module_semantic_impl()
	{
		clean_node_semantics();
	}

	virtual symbol_ptr root_symbol() const
	{
		return root_symbol_;
	}

	virtual program_ptr root_program() const
	{
		return root_symbol_->node()->as_handle<program>();
	}

	virtual pety_t* pety() const
	{
		return pety_.get();
	}

	virtual diag_chat_ptr diags() const
	{
		return diag_chat_;
	}

	virtual vector<symbol*> const&	global_vars() const
	{
		return global_vars_;
	}

	virtual vector<symbol*>& global_vars()
	{
		return global_vars_;
	}

	virtual vector<symbol*> const& functions() const
	{
		return functions_;
	}

	virtual vector<symbol*>& functions()
	{
		return functions_;
	}

	virtual vector<symbol*> const& intrinsics() const
	{
		return intrinsics_;
	}

	virtual vector<symbol*>& intrinsics()
	{
		return intrinsics_;
	}

	virtual node_semantic* get(node const& v) const
	{
		unordered_map<node const*, node_semantic*>::const_iterator it
			= semantics_dict_.find(&v);
		if( it == semantics_dict_.end() ){ return NULL; }
		return it->second;
	}

	virtual node_semantic* get_or_create( node const& v )
	{
		node_semantic* ret = get(v);
		if( ret == NULL )
		{
			ret = new_node_sem();
			semantics_dict_.insert( make_pair(&v, ret) );
		}
		return ret;
	}

private:
	node_semantic* new_node_sem()
	{
		node_semantic* ret = static_cast<node_semantic*>( node_semantic_pool_.malloc() );
		memset( ret, 0, sizeof(node_semantic) );
		ret->owner(this); 
		return ret;
	}

	void clean_node_semantics()
	{
		for(vector<node_semantic*>::iterator it = semantics_.begin(); it != semantics_.end(); ++it )
		{
			(*it)->~node_semantic();
		}
	}

	pety_t_ptr		pety_;
	symbol_ptr		root_symbol_;
	diag_chat_ptr	diag_chat_;

	vector<symbol*> global_vars_;
	vector<symbol*> functions_;
	vector<symbol*> intrinsics_;

	boost::pool<> node_semantic_pool_;
	vector<node_semantic*> semantics_;
	unordered_map<node const*, node_semantic*> semantics_dict_;
};

string const& node_semantic::function_name() const
{
	if( !function_name_ )
	{ 
		const_cast<node_semantic*>(this)->function_name_
			= new string();
	}
	return *function_name_;
}

void node_semantic::function_name( std::string const& v )
{
	if( !function_name_ )
	{ 
		function_name_ = new string(v);
	}
	else
	{
		*function_name_ = v;
	}
}

node_semantic::labeled_statement_array const&
	node_semantic::labeled_statements() const
{
	return const_cast<node_semantic*>(this)->labeled_statements();
}

node_semantic::labeled_statement_array& node_semantic::labeled_statements()
{
	if( !labeled_statements_ )
	{
		labeled_statements_ = new labeled_statement_array();
	}
	return *labeled_statements_;
}

void node_semantic::semantic_value( salviar::semantic_value const& v )
{
	if( !semantic_value_ )
	{
		semantic_value_ = new salviar::semantic_value(v);
	}
	else
	{
		*semantic_value_ = v;
	}
}

node_semantic::~node_semantic()
{
	if( semantic_value_ )
	{
		delete semantic_value_;
		semantic_value_ = NULL;
	}

	if( function_name_ )
	{
		delete function_name_;
		function_name_ = NULL;
	}

	if( labeled_statements_ )
	{
		delete labeled_statements_;
		labeled_statements_ = NULL;
	}
}


module_semantic_ptr module_semantic::create()
{
	return module_semantic_ptr( new module_semantic_impl() );
}

END_NS_SASL_SEMANTIC();