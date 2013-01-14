#include <sasl/include/shims/ia_shim.h>

#include <salviar/include/input_layout.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/functional/hash.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_SHIMS();

size_t hash_value(ia_shim_key const& key)
{
	size_t seed = 0;

	boost::hash_combine(seed, key.input);		// Input layout we need to hash all fields.
	boost::hash_combine(seed, key.reflection);	// Shader layout we just use address as hash code.
	
	return seed;
}

ia_shim_ptr ia_shim::create()
{
	return ia_shim_ptr( new ia_shim() );
}

void* ia_shim::get_shim_function(
		salviar::input_layout_ptr const&	input,
		salviar::shader_reflection const*	reflection
	)
{
	cached_shim_function_dict::iterator iter = cached_shim_funcs_.find( ia_shim_key(input, reflection) );
	if (iter != cached_shim_funcs_.end() )
	{
		return iter->second;
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}


END_NS_SASL_SHIMS();