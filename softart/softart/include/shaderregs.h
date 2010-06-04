#ifndef SOFTART_SHADERREGS_H
#define SOFTART_SHADERREGS_H

#include "decl.h"
#include "colors.h"
#include "surface.h"
#include "renderer_capacity.h"

#include "eflib/include/math.h"

#ifdef EFLIB_MSVC
#pragma warning(push)
#pragma warning(disable : 6385)
#endif
#include <boost/array.hpp>
#ifdef EFLIB_MSVC
#pragma warning(pop)
#endif
#include "softart_fwd.h"
BEGIN_NS_SOFTART()


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
	friend vs_output& integral_unproject(vs_output& out, const vs_output& in, float step, const vs_output& derivation);

	friend void update_wpos(vs_output& vso, const viewport& vp);

	friend float compute_area(const vs_output& v0, const vs_output& v1, const vs_output& v2);

public:
	enum attrib_modifier_type
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
	bool front_face;

	attrib_array_type attributes;
	attrib_modifier_array_type attribute_modifiers;

	uint32_t num_used_attribute;

public:
	vs_output()
		:position(0, 0, 0, 0), front_face(true), num_used_attribute(0)
	{}
	vs_output(
		const efl::vec4& position, 
		bool front_face,
		const attrib_array_type& attribs,
		const attrib_modifier_array_type& modifiers,
		uint32_t num_used_attrib)
		:position(position), front_face(front_face), attributes(attribs), attribute_modifiers(modifiers),
			num_used_attribute(num_used_attrib)
	{}

	//拷贝构造与赋值
	vs_output(const vs_output& rhs)
		:position(rhs.position), front_face(rhs.front_face),
			num_used_attribute(rhs.num_used_attribute)
	{
		for (uint32_t i = 0; i < num_used_attribute; ++ i){
			attributes[i] = rhs.attributes[i];
			attribute_modifiers[i] = rhs.attribute_modifiers[i];
		}
	}

	vs_output& operator = (const vs_output& rhs){
		if(&rhs == this) return *this;
		position = rhs.position;
		front_face = rhs.front_face;
		num_used_attribute = rhs.num_used_attribute;
		for (uint32_t i = 0; i < num_used_attribute; ++ i){
			attributes[i] = rhs.attributes[i];
			attribute_modifiers[i] = rhs.attribute_modifiers[i];
		}
		return *this;
	}

public:
	vs_output& operator+=(const vs_output& rhs);
	vs_output& operator-=(const vs_output& rhs);
	vs_output& operator*=(float f);
	vs_output& operator/=(float f);
};

//vs_output compute_derivate
struct ps_output
{
	float depth;
	boost::array<efl::vec4, pso_color_regcnt> color;
	bool front_face;
};

struct backbuffer_pixel_out
{
	backbuffer_pixel_out(std::vector<surface*>& cbuf, surface* dbuf, surface* sbuf)
		:cbuf_(&cbuf), dbuf_(dbuf), sbuf_(sbuf) {
	}

	void set_pos(size_t x, size_t y){
		buf_x_ = x;
		buf_y_ = y;
	}

	color_rgba32f color(size_t regidx) const{
		return (*cbuf_)[regidx]->get_texel(buf_x_, buf_y_);
	}
	float depth() const{
		return dbuf_->get_texel(buf_x_, buf_y_).r;
	}
	int32_t stencil() const{
		return int32_t(sbuf_->get_texel(buf_x_, buf_y_).r);
	}

	void color(size_t regidx, const color_rgba32f& clr){
		(*cbuf_)[regidx]->set_texel(buf_x_, buf_y_, clr);
	}

	void depth(float depth){
		dbuf_->set_texel(buf_x_, buf_y_, color_rgba32f(depth, 0, 0, 0));
	}

	void stencil(int32_t stencil){
		sbuf_->set_texel(buf_x_, buf_y_, color_rgba32f(float(stencil), 0, 0, 0));
	}

private:
	backbuffer_pixel_out(const backbuffer_pixel_out& rhs);
	backbuffer_pixel_out& operator = (const backbuffer_pixel_out& rhs);

	std::vector<surface*>* cbuf_;
	surface* dbuf_;
	surface* sbuf_;
	size_t buf_x_, buf_y_;
};
END_NS_SOFTART()

#endif