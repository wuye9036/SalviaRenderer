#include <sasl/include/semantic/deps_graph.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

using boost::shared_ptr;
using boost::make_shared;

BEGIN_NS_SASL_SEMANTIC();

shared_ptr<deps_graph> deps_graph::create()
{
	return shared_ptr<deps_graph>( new deps_graph() );
}

END_NS_SASL_SEMANTIC();