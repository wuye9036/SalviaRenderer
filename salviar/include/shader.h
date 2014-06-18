#pragma once

#include <salviar/include/salviar_forward.h>

#include <salviar/include/shader_utility.h>
#include <salviar/include/sampler.h>
#include <salviar/include/enums.h>
#include <salviar/include/renderer_capacity.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/pointer_cast.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>
#include <string>
#include <map>

BEGIN_NS_SALVIAR();

struct viewport;
struct scanline_info;
struct pixel_accessor;
struct triangle_info;
class  vs_input;
class  vs_output;
struct ps_output;

EFLIB_DECLARE_CLASS_SHARED_PTR(cpp_shader);
EFLIB_DECLARE_CLASS_SHARED_PTR(sampler);

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

	semantic_value advance_index(size_t i) const
	{
		semantic_value ret;
		ret.name	= name;
		ret.sv		= sv;
		ret.index	= static_cast<uint32_t>(index + i);
		return ret;
	}

	bool valid() const
	{
		return sv != sv_none || !name.empty();
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

class cpp_shader
{
public:
	virtual result set_sampler(const std::_tstring& varname, sampler_ptr const& samp) = 0;
	virtual result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval) = 0;
	virtual result set_constant(const std::_tstring& varname, shader_constant::const_voidptr pval, size_t index) = 0;

	virtual result find_register( semantic_value const& sv, size_t& index ) = 0;
	virtual boost::unordered_map<semantic_value, size_t> const& get_register_map() = 0;

    template <typename T>
    boost::shared_ptr<T> clone()
    {
        auto ret = boost::dynamic_pointer_cast<T>( clone() );
        assert(ret);
        return ret;
    }

    virtual cpp_shader_ptr clone() = 0;
	virtual ~cpp_shader(){}
};

class cpp_shader_impl : public cpp_shader
{
public:
	result set_sampler(std::_tstring const& samp_name, sampler_ptr const& samp)
	{
		auto samp_it = sampmap_.find(samp_name);
		if( samp_it == sampmap_.end() )
		{
			return result::failed;
		}
		*(samp_it->second) = samp;
		return result::ok;
	}

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

	result declare_sampler(const std::_tstring& varname, sampler_ptr& var)
	{
		sampmap_[varname] = &var;
		return result::ok;
	}

	template<class T>
	result declare_container_constant(const std::_tstring& varname, T& var)
	{
		return declare_container_constant_impl(varname, var, var[0]);
	}

private:
	typedef std::map<std::_tstring, shader_constant::voidptr>
		variable_map;
	typedef std::map<std::_tstring, sampler_ptr*>
		sampler_map;
	typedef std::map<std::_tstring, boost::shared_ptr<detail::container> >
		container_variable_map;
	typedef boost::unordered_map<semantic_value, size_t>
		register_map;

	variable_map			varmap_;
	container_variable_map	contmap_;
	register_map			regmap_;
	sampler_map				sampmap_;

	template<class T, class ElemType>
	result declare_container_constant_impl(const std::_tstring& varname, T& var, const ElemType&)
	{
		varmap_[varname] = shader_constant::voidptr(&var);
		contmap_[varname] = boost::shared_ptr<detail::container>(new detail::container_impl<T, ElemType>(var));
		return result::ok;
	}
};

class cpp_vertex_shader : public cpp_shader_impl
{
public:
	void execute(const vs_input& in, vs_output& out);
	virtual void shader_prog(const vs_input& in, vs_output& out) = 0;
	virtual uint32_t num_output_attributes() const = 0;
	virtual uint32_t output_attribute_modifiers(uint32_t index) const = 0;
};

class cpp_pixel_shader : public cpp_shader_impl
{
	bool					front_face_;
	vs_output const*		px_;
	vs_output const*		quad_;
	uint64_t				lod_flag_;
	float					lod_[MAX_VS_OUTPUT_ATTRS];

protected:
	bool front_face() const	{ return front_face_; }

	eflib::vec4	  ddx(size_t iReg) const;
	eflib::vec4   ddy(size_t iReg) const;

	color_rgba32f tex2d(const sampler& s, size_t iReg);
	color_rgba32f tex2dlod(const sampler& s, size_t iReg);
    color_rgba32f tex2dlod(sampler const& s, eflib::vec4 const& coord_with_lod);
	color_rgba32f tex2dproj(const sampler& s, size_t iReg);

	color_rgba32f texcube(const sampler& s, const eflib::vec4& coord, const eflib::vec4& ddx, const eflib::vec4& ddy, float bias = 0);
	color_rgba32f texcube(const sampler&s, size_t iReg);
	color_rgba32f texcubelod(const sampler& s, size_t iReg);
	color_rgba32f texcubeproj(const sampler& s, size_t iReg);
	color_rgba32f texcubeproj(const sampler&s, const eflib::vec4& v, const eflib::vec4& ddx, const eflib::vec4& ddy);

public:
	void update_front_face(bool v)
	{
		front_face_ = v;
	}

	uint64_t execute(vs_output const* quad_in, ps_output* px_out, float* depth);
	
	virtual bool shader_prog(vs_output const& in, ps_output& out) = 0;
    virtual bool output_depth() const;
};

//it is called when render a shaded pixel into framebuffer
class cpp_blend_shader : public cpp_shader_impl
{
public:
	void execute(size_t sample, pixel_accessor& inout, const ps_output& in);
	virtual bool shader_prog(size_t sample, pixel_accessor& inout, const ps_output& in) = 0;
};

END_NS_SALVIAR();
