#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader_utility.h>
#include <salviar/include/sampler.h>
#include <salviar/include/enums.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>
#include <string>
#include <map>

BEGIN_NS_SALVIAR();

struct viewport;
struct scanline_info;
struct backbuffer_pixel_out;
class  triangle_info;
class  vs_input;
class  vs_output;
struct ps_output;

enum languages
{
	lang_none,
	
	lang_general,
	lang_vertex_shader,
	lang_pixel_shader,
	lang_blending_shader,

	lang_count
};

enum system_values
{
	sv_none,

	sv_position,
	sv_texcoord,
	sv_normal,

	sv_blend_indices,
	sv_blend_weights,

	sv_target,
	sv_depth,

	sv_customized
};

class semantic_value{
public:
	static std::string lower_copy( std::string const& name ){
		std::string ret(name);
		for( size_t i = 0; i < ret.size(); ++i ){
			if( 'A' <= ret[i] && ret[i] <= 'Z' ){
				ret[i] = ret[i] - ('A' - 'a');
			}
		}
		return ret;
	}

	semantic_value(): sv(sv_none), index(0){}

	explicit semantic_value( std::string const& name, uint32_t index = 0 ){
		assert( !name.empty() );

		std::string lower_name = lower_copy(name);

		if( lower_name == "position" || lower_name == "sv_position" ){
			sv = sv_position;
		} else if ( lower_name == "normal" ){
			sv = sv_normal;
		} else if ( lower_name == "texcoord" ){
			sv = sv_texcoord;
		} else if ( lower_name == "color" || lower_name == "sv_target" ){
			sv = sv_target;
		} else if ( lower_name == "depth" || lower_name == "sv_depth" ) {
			sv = sv_depth;
		} else if ( lower_name == "blend_indices" ){
			sv = sv_blend_indices;
		} else if ( lower_name == "blend_weights" ){
			sv = sv_blend_weights;
		} else {
			sv = sv_customized;
			this->name = lower_name;
		}
		this->index = index;
	}

	semantic_value( system_values sv, uint32_t index = 0 ){
		assert( sv_none <= sv && sv < sv_customized );
		this->sv = sv;
		this->index = index;
	}

	std::string const& get_name() const{
		return name;
	}

	system_values const& get_system_value() const{
		return sv;
	}

	uint32_t get_index() const{
		return index;
	}

	bool operator < ( semantic_value const& rhs ) const{
		return sv < rhs.sv || name < rhs.name || index < rhs.index;
	}

	bool operator == ( semantic_value const& rhs ) const{
		return is_same_sv(rhs) && index == rhs.index;
	}

	bool operator == ( system_values rhs ) const{
		return sv == rhs && index == 0;
	}

	template <typename T>
	bool operator != ( T const& v ) const{
		return !( *this == v );
	}

private:
	std::string		name;
	system_values	sv;
	uint32_t		index;

	bool is_same_sv( semantic_value const& rhs ) const{
		if( sv != rhs.sv ) return false;
		if( sv == sv_customized ) return rhs.name == name;
		return true;
	}
};

inline size_t hash_value( semantic_value const& v ){
	size_t seed = v.get_index();
	if(v.get_system_value() != sv_customized )
	{
		boost::hash_combine( seed, static_cast<size_t>( v.get_system_value() ) );
	}
	else
	{
		boost::hash_combine( seed, v.get_name() );
	}
	return seed;
}

struct shader_profile
{
	languages language;
};

class shader
{
public:
	virtual result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval) = 0;
	virtual result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval, size_t index) = 0;

	virtual result find_register( semantic_value const& sv, size_t& index ) = 0;
	virtual boost::unordered_map<semantic_value, size_t> const& get_register_map() = 0;

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

	result find_register( semantic_value const& sv, size_t& index );
	boost::unordered_map<semantic_value, size_t> const& get_register_map();
	void bind_semantic( char const* name, size_t semantic_index, size_t register_index );
	void bind_semantic( semantic_value const& s, size_t register_index );

	template< class T >
	result declare_constant(const std::_tstring& varname, T& var)
	{
		varmap_[varname] = shader_constant::voidptr(&var);
		return result::ok;
	}

	template<class T>
	result declare_container_constant(const std::_tstring& varname, T& var)
	{
		return declare_container_constant_impl(varname, var, var[0]);
	}

private:
	typedef std::map<std::_tstring, shader_constant::voidptr> variable_map;
	typedef std::map<std::_tstring, boost::shared_ptr<detail::container> > container_variable_map;
	typedef boost::unordered_map<semantic_value, size_t> register_map;

	variable_map varmap_;
	container_variable_map contmap_;
	register_map regmap_;

	template<class T, class ElemType>
	result declare_container_constant_impl(const std::_tstring& varname, T& var, const ElemType&)
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
	virtual uint32_t num_output_attributes() const = 0;
	virtual uint32_t output_attribute_modifiers(uint32_t index) const = 0;
};

class pixel_shader : public shader_impl
{
	friend class rasterizer;
	
	const triangle_info* ptriangleinfo_;
	const vs_output* ppxin_;

protected:
	const eflib::vec4& get_pos_ddx() const;
	const eflib::vec4& get_pos_ddy() const;

	// unproj_dd? = dd?(proj_attr)
	const eflib::vec4& unproj_ddx(size_t iReg) const;
	const eflib::vec4& unproj_ddy(size_t iReg) const;

	// dd? = dd?(proj_attr/proj_attr.w)
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

	virtual pixel_shader_ptr create_clone() = 0;
	virtual void destroy_clone(pixel_shader_ptr& ps_clone) = 0;
};

//it is called when render a shaded pixel into framebuffer
class blend_shader : public shader_impl
{
public:
	void execute(size_t sample, backbuffer_pixel_out& inout, const ps_output& in);
	virtual bool shader_prog(size_t sample, backbuffer_pixel_out& inout, const ps_output& in) = 0;
};

END_NS_SALVIAR();