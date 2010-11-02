#ifndef SOFTART_SHADER_H
#define SOFTART_SHADER_H

#include "shader_utility.h"
#include "shaderregs.h"
#include "sampler.h"
#include "renderer_capacity.h"
#include "enums.h"

#include <eflib/include/platform/disable_warnings.h>
#include <boost/smart_ptr.hpp>
#include <boost/array.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>
#include <map>
#include "softart_fwd.h"

BEGIN_NS_SOFTART();

struct viewport;
struct scanline_info;
class triangle_info;

class shader
{
public:
	virtual result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval) = 0;
	virtual result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval, size_t index)	= 0;
	virtual ~shader(){}
};

class shader_impl : public shader
{
public:
	result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval){
		variable_map::iterator var_it = varmap_.find(varname);
		if( var_it == varmap_.end() ){
			return result::failed;
		}
		if(shader_constant::assign(var_it->second, pval)){
			return result::ok;
		}
		return result::failed;
	}

	result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval, size_t index)
	{
		container_variable_map::iterator cont_it = contmap_.find(varname);
		if( cont_it == contmap_.end() ){
			return result::failed;
		}
		cont_it->second->set(pval, index);
		return result::ok;
	}

	template<class T>
	result register_var(const std::_tstring& varname, T& var)
	{
		varmap_[varname] = shader_constant::voidptr(&var);
		return result::ok;
	}

	template<class T>
	result register_var_as_container(const std::_tstring& varname, T& var)
	{
		return register_var_as_container_impl(varname, var, var[0]);
	}

private:
	typedef std::map<std::_tstring, shader_constant::voidptr> variable_map;
	typedef std::map<std::_tstring, boost::shared_ptr<detail::container> > container_variable_map;

	variable_map varmap_;
	container_variable_map contmap_;

	template<class T, class ElemType>
	result register_var_as_container_impl(const std::_tstring& varname, T& var, const ElemType&)
	{
		varmap_[varname] = shader_constant::voidptr(&var);
		contmap_[varname] = boost::shared_ptr<detail::container>(new detail::container_impl<T, ElemType>(var));
		return result::ok;
	}
};

class vertex_shader : public shader_impl
{
public:
	void execute(const vs_input& in, vs_output& out);
	virtual void shader_prog(const vs_input& in, vs_output& out) = 0;
};

class pixel_shader : public shader_impl
{
	friend class rasterizer;
	
	const triangle_info* ptriangleinfo_;
	const vs_output* ppxin_;

protected:
	const eflib::vec4& get_pos_ddx() const;
	const eflib::vec4& get_pos_ddy() const;

	//获得乘以投影系数之后的ddx与ddy，可以用它计算投影纠正后的ddx和ddy。
	const eflib::vec4& get_original_ddx(size_t iReg) const;
	const eflib::vec4& get_original_ddy(size_t iReg) const;

	//获得投影纠正以后的ddx与ddy
	const eflib::vec4 ddx(size_t iReg) const;
	const eflib::vec4 ddy(size_t iReg) const;

	color_rgba32f tex2d(const sampler& s, const eflib::vec4& coord, const eflib::vec4& ddx, const eflib::vec4& ddy, float bias = 0);
	color_rgba32f tex2d(const sampler& s, size_t iReg);
	color_rgba32f tex2dlod(const sampler& s, size_t iReg);
	color_rgba32f tex2dproj(const sampler& s, size_t iReg);
	color_rgba32f tex2dproj(const sampler& s, const eflib::vec4& v, const eflib::vec4& ddx, const eflib::vec4& ddy);

	color_rgba32f texcube(const sampler& s, const eflib::vec4& coord, const eflib::vec4& ddx, const eflib::vec4& ddy, float bias = 0);
	color_rgba32f texcube(const sampler&s, size_t iReg);
	color_rgba32f texcubelod(const sampler& s, size_t iReg);
	color_rgba32f texcubeproj(const sampler& s, size_t iReg);
	color_rgba32f texcubeproj(const sampler&s, const eflib::vec4& v, const eflib::vec4& ddx, const eflib::vec4& ddy);
public:
	bool execute(const vs_output& in, ps_output& out);
	virtual bool shader_prog(const vs_output& in, ps_output& out) = 0;

	virtual h_pixel_shader create_clone() = 0;
	virtual void destroy_clone(h_pixel_shader& ps_clone) = 0;
};

//it is called when render a shaded pixel into framebuffer
class blend_shader : public shader_impl
{
public:
	void execute(size_t sample, backbuffer_pixel_out& inout, const ps_output& in);
	virtual bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in) = 0;
};

END_NS_SOFTART()

#endif