#include <sasl/include/semantic/semantic_error.h>

BEGIN_NS_SASL_SEMANTIC_ERRORS();

using ::sasl::syntax_tree::node;
using ::boost::shared_ptr;
using ::std::string;
using ::std::vector;

void semantic_error::add_refnode( shared_ptr<node> refnode ){
	refnodes.push_back(refnode);
}

std::vector<shared_ptr<node> >& semantic_error::ref_nodes(){
	return refnodes;
}

shared_ptr<semantic_error> semantic_error::create(
	compiler_informations info,
	shared_ptr<node> error_node,
	vector<shared_ptr<node> > refnodes
	){
	return boost::shared_ptr<semantic_error>( new semantic_error( info, error_node, refnodes ) );
}

vector< shared_ptr<node> > semantic_error::new_ref_node_list() {
	return vector< shared_ptr<node> >();
}

semantic_error::semantic_error( compiler_informations info,
		shared_ptr<node> error_node,
		vector< shared_ptr<node> > refnodes
		): compiler_information_impl( info ), error_node( error_node ), refnodes(refnodes){}

string semantic_error::desc(){
	return id_str() + string(":") + compiler_informations::to_name( info_id ) ;
}
END_NS_SASL_SEMANTIC_ERRORS();
