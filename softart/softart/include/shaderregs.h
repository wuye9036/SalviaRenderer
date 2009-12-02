#ifndef SOFTART_SHADERREGS_H
#define SOFTART_SHADERREGS_H

#include "colors.h"
#include "renderer_capacity.h"

#include "eflib/include/math.h"

#include <boost/array.hpp>

class vs_input
{
public:
	typedef boost::array<efl::vec4, vsi_attrib_regcnt> vsinput_attributes_t;

	const efl::vec4& operator [](size_t i) const;
	efl::vec4& operator[](size_t i);

protected:
	vsinput_attributes_t attributes_;
	
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
};

class vs_output
{
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

	uint32_t used_attribute_mask;

protected:
	vs_output(){}
	vs_output(
		const efl::vec4& position, 
		const efl::vec4& wpos,
		const attrib_array_type& attribs,
		const attrib_modifier_array_type& modifiers,
		uint32_t used_attrib_mask)
		:position(position), wpos(wpos), attributes(attribs), attribute_modifiers(modifiers),
			used_attribute_mask(used_attrib_mask)
	{}

	//拷贝构造与赋值
	vs_output(const vs_output& rhs)
		:position(rhs.position), wpos(rhs.wpos), attributes(rhs.attributes), attribute_modifiers(rhs.attribute_modifiers),
			used_attribute_mask(rhs.used_attribute_mask)
	{}

	vs_output& operator = (const vs_output& rhs){
		if(&rhs == this) return *this;
		position = rhs.position;
		wpos = rhs.wpos;
		attributes = rhs.attributes;
		attribute_modifiers = rhs.attribute_modifiers;
		used_attribute_mask = rhs.used_attribute_mask;
		return *this;
	}
};

//vs_output compute_derivate
struct ps_output
{
	efl::vec4 pos;
	boost::array<efl::vec4, pso_color_regcnt> color;
	
	ps_output(){}
};

struct backbuffer_pixel_in
{
	backbuffer_pixel_in(const boost::array<const color_rgba32f*, pso_color_regcnt>& pcolors, const float* depth, const size_t* stencil)
		:pcolor_(pcolors), pdepth_(depth), pstencil_(stencil){
	}

	const color_rgba32f& color(size_t regidx) const{
		return *pcolor_[regidx];
	}

	float depth() const{
		return *pdepth_;
	}

	size_t stencil() const{
		return *pstencil_;
	}

private:
	backbuffer_pixel_in(const backbuffer_pixel_in& rhs);
	backbuffer_pixel_in& operator = (const backbuffer_pixel_in& rhs);

	boost::array<const color_rgba32f*, pso_color_regcnt> pcolor_;
	const float* pdepth_;
	const size_t* pstencil_;
};

struct backbuffer_pixel_out
{
	backbuffer_pixel_out(const boost::array<color_rgba32f*, pso_color_regcnt>& pcolors, float* depth, size_t* stencil)
		:pcolor_(pcolors), pdepth_(depth), pstencil_(stencil){
			is_color_rewritten_.assign(false);
	}

	const color_rgba32f& color(size_t regidx) const{ return *pcolor_[regidx];}
	float depth() const { return *pdepth_; }
	size_t stencil() const { return *pstencil_; }

	color_rgba32f& color(size_t regidx){
		is_color_rewritten_[regidx] = true;
		return *pcolor_[regidx];
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
	boost::array<color_rgba32f*, pso_color_regcnt> pcolor_;
	float* pdepth_;
	size_t* pstencil_;
};
#endif