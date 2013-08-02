#pragma once

#include <salviax/include/salviax_forward.h>
#include <eflib/include/utility/shared_declaration.h>

namespace salviar
{
	EFLIB_DECLARE_CLASS_SHARED_PTR(surface);
	EFLIB_DECLARE_CLASS_SHARED_PTR(renderer);
	class renderer_parameters;
}

BEGIN_NS_SALVIAX();

EFLIB_DECLARE_CLASS_SHARED_PTR(swap_chain);
class swap_chain
{
public:
	virtual salviar::surface_ptr	get_surface()	= 0;
	virtual void					present()		= 0;
};

enum swap_chain_types
{
	swap_chain_d3d11= 1UL,
	swap_chain_gl	= 2UL
};

enum renderer_types
{
	renderer_async	= 1UL,
	renderer_sync	= 2UL
};

END_NS_SALVIAX();

#ifdef salviax_swap_chain_EXPORTS
	#define SALVIAX_SWAP_CHAIN_API __declspec(dllexport)
#else
	#define SALVIAX_SWAP_CHAIN_API __declspec(dllimport)
#endif

extern "C"
{
	SALVIAX_SWAP_CHAIN_API void salviax_create_swap_chain_and_renderer(
		salviax::swap_chain_ptr&			out_swap_chain,
		salviar::renderer_ptr&				out_renderer,
		salviar::renderer_parameters const*	render_params,
		uint32_t							renderer_type	= salviax::swap_chain_gl,
		uint32_t		 					swap_chain_type	= salviax::swap_chain_d3d11
	);
}