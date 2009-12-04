#ifndef SOFTART_SHADERREGS_H
#define SOFTART_SHADERREGS_H

#include "decl.h"
#include "colors.h"
#include "surface.h"
#include "renderer_capacity.h"

#include "eflib/include/math.h"

#include <boost/array.hpp>

class vs_input
{
public:
	typedef boost::array<efl::vec4, vsi_attrib_regcnt> vsinput_attributes_t;

	const efl::vec4& operator [](size_t i) const;
	efl::vec4& operator[](size_t i);

	vs_input(){}
	
	vs_input(vsinput_attributes_t& attrs)
		:attributes_(attrs)
	{}

	vs_input(const vs_input& rhs)
		:attributes_(rhs.attributes_)
	{}

	vs_input& operator = (const vs_input& rhs)
	{
		attributes_ = rhs.attributes_;
	}

protected:
	vsinput_attributes_t attributes_;
};

class vs_output
{
	friend vs_output operator + (const vs_output& vso0, const vs_output& vso1);
	friend vs_output operator - (const vs_output& vso0, const vs_output& vso1);
	friend vs_output operator * (const vs_output& vso0, float f);
	friend vs_output operator * (float f, const vs_output& vso0);
	friend vs_output operator / (const vs_output& vso0, float f);

	friend vs_output project(const vs_output& in);
	friend vs_output& project(vs_output& out, const vs_output& in);
	friend vs_output unproject(const vs_output& in);
	friend vs_output& unproject(vs_output& out, const vs_output& in);

	friend vs_output lerp(const vs_output& start, const vs_output& end, float step);
	friend vs_output& integral(vs_output& inout, float step, const vs_output& derivation);

	friend void update_wpos(vs_output& vso, const viewport& vp);

	friend float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2);

public:
	enum
	{
		am_linear = 1UL << 0,
		am_nointerpolation = 1UL << 1,
		am_noperspective = 1UL << 2,
		am_sample = 1UL << 3
	};

public:
	typedef boost::array<efl::vec4, vso_attrib_regcnt> attrib_array_type;
	typedef boost::array<uint32_t, vso_attrib_regcnt> attrib_modifier_array_type;

	efl::vec4 position;
	efl::vec4 wpos;

	attrib_array_type attributes;
	attrib_modifier_array_type attribute_modifiers;

	uint32_t num_used_attribute;

public:
	vs_output(){}
	vs_output(
		const efl::vec4& position, 
		const efl::vec4& wpos,
		const attrib_array_type& attribs,
		const attrib_modifier_array_type& modifiers,
		uint32_t num_used_attrib)
		:position(position), wpos(wpos), attributes(attribs), attribute_modifiers(modifiers),
			num_used_attribute(num_used_attrib)
	{}

	//拷贝构造与赋值
	vs_output(const vs_output& rhs)
		:position(rhs.position), wpos(rhs.wpos), attributes(rhs.attributes), attribute_modifiers(rhs.attribute_modifiers),
			num_used_attribute(rhs.num_used_attribute)
	{}

	vs_output& operator = (const vs_output& rhs){
		if(&rhs == this) return *this;
		position = rhs.position;
		wpos = rhs.wpos;
		attributes = rhs.attributes;
		attribute_modifiers = rhs.attribute_modifiers;
		num_used_attribute = rhs.num_used_attribute;
		return *this;
	}
};

//vs_output compute_derivate
struct ps_output
{
	float depth;
	boost::array<efl::vec4, pso_color_regcnt> color;
};

struct backbuffer_pixel_in
{
	backbuffer_pixel_in(const ps_output& ps, const size_t* stencil)
		:ps_(&ps), pstencil_(stencil){
	}

	const color_rgba32f& color(size_t regidx) const{
		return *reinterpret_cast<const color_rgba32f*>(&ps_->color[regidx]);
	}

	float depth() const{
		return ps_->depth;
	}

	size_t stencil() const{
		return *pstencil_;
	}

private:
	backbuffer_pixel_in(const backbuffer_pixel_in& rhs);
	backbuffer_pixel_in& operator = (const backbuffer_pixel_in& rhs);

	const ps_output* ps_;
	const size_t* pstencil_;
};

struct backbuffer_pixel_out
{
	backbuffer_pixel_out(std::vector<surface*>& cbuf, size_t x, size_t y, float* depth, size_t* stencil)
		:cbuf_(&cbuf), cbuf_x_(x), cbuf_y_(y), pdepth_(depth), pstencil_(stencil){
			is_color_rewritten_.assign(false);
	}

	color_rgba32f color(size_t regidx) const{
		return (*cbuf_)[regidx]->get_texel(cbuf_x_, cbuf_y_);
	}
	float depth() const { return *pdepth_; }
	size_t stencil() const { return *pstencil_; }

	void color(size_t regidx, const color_rgba32f& clr){
		is_color_rewritten_[regidx] = true;
		(*cbuf_)[regidx]->set_texel(cbuf_x_, cbuf_y_, clr);
	}

	float& depth(){
		return *pdepth_;
	}

	size_t& stencil(){
		return *pstencil_;
	}

private:
	backbuffer_pixel_out(const backbuffer_pixel_out& rhs);
	backbuffer_pixel_out& operator = (const backbuffer_pixel_out& rhs);

	boost::array<bool, pso_color_regcnt> is_color_rewritten_;
	std::vector<surface*>* cbuf_;
	size_t cbuf_x_, cbuf_y_;
	float* pdepth_;
	size_t* pstencil_;
};
#endif