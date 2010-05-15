#include <sasl/include/semantic/semantic_error.h>

BEGIN_NS_SASL_SEMANTIC_ERRORS();

using ::sasl::syntax_tree::node;

void semantic_error::add_refnode( boost::shared_ptr<node> refnode ){
	refnodes.push_back(refnode);
}

std::vector<boost::shared_ptr<node> >& semantic_error::ref_nodes(){
	return refnodes;
}

boost::shared_ptr<semantic_error> semantic_error::create(
	compiler_informations info,
	boost::shared_ptr<::sasl::syntax_tree::node> error_node,
	std::vector< boost::shared_ptr<::sasl::syntax_tree::node> > refnodes
	){
	return boost::shared_ptr<semantic_error>( new semantic_error( info, error_node, refnodes ) );
}

std::vector< boost::shared_ptr<node> > semantic_error::new_ref_node_list() {
	return std::vector< boost::shared_ptr<node> >();
}

semantic_error::semantic_error( compiler_informations info,
		boost::shared_ptr<::sasl::syntax_tree::node> error_node,
		std::vector< boost::shared_ptr<::sasl::syntax_tree::node> > refnodes
		): compiler_information_impl( info ), error_node( error_node ), refnodes(refnodes){}

std::string semantic_error::desc(){
	return id_str() + std::string(":") + compiler_informations::to_name( info_id ) ;
}
END_NS_SASL_SEMANTIC_ERRORS();