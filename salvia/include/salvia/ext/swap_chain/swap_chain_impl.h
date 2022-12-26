#pragma once

#include <salviax/include/salviax_forward.h>
#include <salviax/include/swap_chain/swap_chain.h>

BEGIN_NS_SALVIAX();

class swap_chain_impl: public swap_chain
{
public:
	virtual salviar::surface_ptr get_surface();
	virtual void				 present();

protected:
	swap_chain_impl(
		salviar::renderer_ptr const& renderer,
		salviar::renderer_parameters const& render_params);
	
	virtual void present_impl() = 0;
	
protected:
	salviar::renderer_ptr	renderer_;
	salviar::surface_ptr	surface_;
	salviar::surface_ptr	resolved_surface_;
};

#if defined(SALVIAX_GL_ENABLED)
swap_chain_ptr create_gl_swap_chain(
	salviar::renderer_ptr const& renderer,
	salviar::renderer_parameters const*);
#endif

#if defined(SALVIAX_D3D11_ENABLED)
swap_chain_ptr create_d3d11_swap_chain(
	salviar::renderer_ptr const& renderer,
	salviar::renderer_parameters const*);
#endif

END_NS_SALVIAX();
