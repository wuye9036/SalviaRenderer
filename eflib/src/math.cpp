#include <eflib/include/math/math.h>

namespace eflib{
	vec2 normalize2(const vec2& v)
	{
		float length = v.length();
		if(equal<float>(length, 0.0f)){
			length = 1.0f;
		}
		float inv_length = 1.0f / length;
		return v * inv_length;
	}

	vec3 normalize3(const vec3& v)
	{
		float length = v.length();
		if(equal<float>(length, 0.0f)){
			length = 1.0f;
		}
		float inv_length = 1.0f / length;
		return v * inv_length;
	}

	vec4 normalize4(const vec4& v)
	{
		float length = v.length();
		if(equal<float>(length, 0.0f)){
			length = 1.0f;
		}
		float inv_length = 1.0f / length;
		return v * inv_length;
	}

	float dot_prod2(const vec2& v1, const vec2& v2)
	{
		return v1[0]*v2[0] + v1[1]*v2[1];
	}

	float dot_prod3(const vec3& v1, const vec3& v2)
	{
		return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
	}

	float dot_prod4(const vec4& v1, const vec4& v2)
	{
		return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3];
	}

	float cross_prod2(const vec2& v1, const vec2& v2)
	{
		return v1[0] * v2[1] - v1[1] * v2[0];
	}

	vec3 cross_prod3(const vec3& v1, const vec3& v2)
	{
		return vec3(
			v1[1]*v2[2] - v1[2]*v2[1],
			v1[2]*v2[0] - v1[0]*v2[2],
			v1[0]*v2[1] - v1[1]*v2[0]
			);
	}

	vec2 clampps(const vec2& v, const vec2& minv, const vec2& maxv)
	{
		return vec2(
			clamp(v[0], minv[0], maxv[0]),
			clamp(v[1], minv[1], maxv[1])
			);
	}

	vec3 clampps(const vec3& v, const vec3& minv, const vec3& maxv)
	{
		return vec3(
			clamp(v[0], minv[0], maxv[0]),
			clamp(v[1], minv[1], maxv[1]),
			clamp(v[2], minv[2], maxv[2])
			);
	}

	vec4 clampps(const vec4& v, const vec4& minv, const vec4& maxv)
	{
		return vec4(
			clamp(v[0], minv[0], maxv[0]),
			clamp(v[1], minv[1], maxv[1]),
			clamp(v[2], minv[2], maxv[2]),
			clamp(v[3], minv[3], maxv[3])
			);
	}

	vec2 clampss(const vec2& v, float min, float max)
	{
		return vec2(
			clamp(v[0], min, max),
			clamp(v[1], min, max)
			);
	}

	vec3 clampss(const vec3& v, float min, float max)
	{
		return vec3(
			clamp(v[0], min, max),
			clamp(v[1], min, max),
			clamp(v[2], min, max)
			);
	}

	vec4 clampss(const vec4& v, float min, float max)
	{
		return vec4(
			clamp(v[0], min, max),
			clamp(v[1], min, max),
			clamp(v[2], min, max),
			clamp(v[3], min, max)
			);
	}

	vec3 reflect3(const vec3& i, const vec3& n)
	{
		return i - n * (2.0f * dot_prod3(i, n));
	}

	vec4 reflect4(const vec4& i, const vec4& n)
	{
		return i - n * (2.0f * dot_prod4(i, n));
	}

	vec3 refract3(const vec3& i, const vec3& n, float eta )
	{
		float n_dot_i = dot_prod3(n, i);
		float k = 1.0f - eta * eta * (1.0f - n_dot_i * n_dot_i);
		if( k < 0 ){
			return vec3(0.0f, 0.0f, 0.0f);
		} else {
			return eta * i - ( eta * n_dot_i + sqrtf(k) ) * n;
		}
	}

	vec4 refract4(const vec4& i, const vec4& n, float eta)
	{
		float n_dot_i = dot_prod4(n, i);
		float k = 1.0f - eta * eta * (1.0f - n_dot_i * n_dot_i);
		if( k < 0 ){
			return vec4(0.0f, 0.0f, 0.0f, 0.0f);
		} else {
			return eta * i - ( eta * n_dot_i + sqrtf(k) ) * n;
		}
	}

	float smoothstep(float min_v, float max_v, float v)
	{
		float t = clamp( (v-min_v)/(max_v-min_v), 0.0f, 1.0f);
		return t * t * (3.0f-2.0f*t);
	}

	vec4& gen_plane(vec4& out, const vec4& v0, const vec4& v1, const vec4& v2)
	{
		vec4 e01 = v1 - v0;
		vec4 e12 = v2 - v1;

		out.xyz() = cross_prod3(e01.xyz(), e12.xyz());
		out[3] = -dot_prod3(v0.xyz(), out.xyz());
		return out;
	}

	vec4& hermite(vec4& out, const vec4& /*v0*/, const vec4& /*v1*/, const vec4& /*v2*/, const vec4& /*v3*/)
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
		return out;
	}

	vec4& cutmull_rom(vec4& out, const vec4& /*v0*/, const vec4& /*v1*/, const vec4& /*v2*/, const vec4& /*v3*/)
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
		return out;
	}

	//////////////////////////////////////////////
	//  blas level 2: matrix - vector
	//////////////////////////////////////////////
	vec4& transform(vec4& out, const vec4& v, const mat44& m)
	{
		if(&out == & v)
		{
			vec4 tmpv(v);
			return transform(out, tmpv, m);
		}

		out[0] = dot_prod4(v, m.get_column(0));
		out[1] = dot_prod4(v, m.get_column(1));
		out[2] = dot_prod4(v, m.get_column(2));
		out[3] = dot_prod4(v, m.get_column(3));

		return out;
	}

	vec4& transform( vec4& out, const mat44& m, const vec4& v )
	{
		if(&out == &v){
			vec4 tmpv(v);
			return transform(out, m, tmpv);
		}

		out[0] = dot_prod4(v, m.get_row(0));
		out[1] = dot_prod4(v, m.get_row(1));
		out[2] = dot_prod4(v, m.get_row(2));
		out[3] = dot_prod4(v, m.get_row(3));

		return out;
	}

	vec4& transform_coord(vec4& out, const vec4& v, const mat44& m)
	{
		vec4 coord = vec4(v[0], v[1], v[2], 1.0f);
		transform(out, coord, m);
		if(out[3] == 0.0f) out[3] = 1.0f;
		out /= out[3];
		return out;
	}

	vec4& transform_normal(vec4& out, const vec4& v, const mat44& m)
	{
		vec4 norm = vec4(v[0], v[1], v[2], 0.0f);
		transform(out, norm, m);
		return out;
	}

	vec4& transform33(vec4& out, const vec4& v, const mat44& m)
	{
		if(&out == & v)
		{
			vec4 tmpv(v);
			return transform33(out, tmpv, m);
		}

		out[0] = dot_prod3(v.xyz(), m.get_column(0).xyz());
		out[1] = dot_prod3(v.xyz(), m.get_column(1).xyz());
		out[2] = dot_prod3(v.xyz(), m.get_column(2).xyz());
		out[3] = 0.0f;

		return out;
	}

	//////////////////////////////////////////////
	// blas level 3: matrix - matrix
	//////////////////////////////////////////////
	mat44& mat_mul(mat44& out, const mat44& m1, const mat44& m2)
	{
		if(&out == &m1)
		{
			mat44 tmpm(m1);
			return mat_mul(out, tmpm, m2);
		}
		if(&out == &m2)
		{
			mat44 tmpm(m2);
			return mat_mul(out, m1, tmpm);
		}

		out.data_[0][0] = dot_prod4(m1.get_row(0), m2.get_column(0));
		out.data_[0][1] = dot_prod4(m1.get_row(0), m2.get_column(1));
		out.data_[0][2] = dot_prod4(m1.get_row(0), m2.get_column(2));
		out.data_[0][3] = dot_prod4(m1.get_row(0), m2.get_column(3));
			
		out.data_[1][0] = dot_prod4(m1.get_row(1), m2.get_column(0));
		out.data_[1][1] = dot_prod4(m1.get_row(1), m2.get_column(1));
		out.data_[1][2] = dot_prod4(m1.get_row(1), m2.get_column(2));
		out.data_[1][3] = dot_prod4(m1.get_row(1), m2.get_column(3));
			
		out.data_[2][0] = dot_prod4(m1.get_row(2), m2.get_column(0));
		out.data_[2][1] = dot_prod4(m1.get_row(2), m2.get_column(1));
		out.data_[2][2] = dot_prod4(m1.get_row(2), m2.get_column(2));
		out.data_[2][3] = dot_prod4(m1.get_row(2), m2.get_column(3));
			
		out.data_[3][0] = dot_prod4(m1.get_row(3), m2.get_column(0));
		out.data_[3][1] = dot_prod4(m1.get_row(3), m2.get_column(1));
		out.data_[3][2] = dot_prod4(m1.get_row(3), m2.get_column(2));
		out.data_[3][3] = dot_prod4(m1.get_row(3), m2.get_column(3));

		return out;
	}

	mat44& mat_transpose(mat44& out, const mat44& m1)
	{
		if(&out == &m1)
		{
			std::swap(out.data_[0][1], out.data_[1][0]);
			std::swap(out.data_[0][2], out.data_[2][0]);
			std::swap(out.data_[0][3], out.data_[3][0]);

			std::swap(out.data_[1][2], out.data_[2][1]);
			std::swap(out.data_[1][3], out.data_[3][1]);

			std::swap(out.data_[2][3], out.data_[3][2]);

			return out;
		}

		out.set_row(0, m1.get_column(0));
		out.set_row(1, m1.get_column(1));
		out.set_row(2, m1.get_column(2));
		out.set_row(3, m1.get_column(3));

		return out;
	}

	mat44& mat_inverse(mat44& out, const mat44& m1)
	{
		float _2132_2231(m1.data_[1][0] * m1.data_[2][1] - m1.data_[1][1] * m1.data_[2][0]);
		float _2133_2331(m1.data_[1][0] * m1.data_[2][2] - m1.data_[1][2] * m1.data_[2][0]);
		float _2134_2431(m1.data_[1][0] * m1.data_[2][3] - m1.data_[1][3] * m1.data_[2][0]);
		float _2142_2241(m1.data_[1][0] * m1.data_[3][1] - m1.data_[1][1] * m1.data_[3][0]);
		float _2143_2341(m1.data_[1][0] * m1.data_[3][2] - m1.data_[1][2] * m1.data_[3][0]);
		float _2144_2441(m1.data_[1][0] * m1.data_[3][3] - m1.data_[1][3] * m1.data_[3][0]);
		float _2233_2332(m1.data_[1][1] * m1.data_[2][2] - m1.data_[1][2] * m1.data_[2][1]);
		float _2234_2432(m1.data_[1][1] * m1.data_[2][3] - m1.data_[1][3] * m1.data_[2][1]);
		float _2243_2342(m1.data_[1][1] * m1.data_[3][2] - m1.data_[1][2] * m1.data_[3][1]);
		float _2244_2442(m1.data_[1][1] * m1.data_[3][3] - m1.data_[1][3] * m1.data_[3][1]);
		float _2334_2433(m1.data_[1][2] * m1.data_[2][3] - m1.data_[1][3] * m1.data_[2][2]);
		float _2344_2443(m1.data_[1][2] * m1.data_[3][3] - m1.data_[1][3] * m1.data_[3][2]);
		float _3142_3241(m1.data_[2][0] * m1.data_[3][1] - m1.data_[2][1] * m1.data_[3][0]);
		float _3143_3341(m1.data_[2][0] * m1.data_[3][2] - m1.data_[2][2] * m1.data_[3][0]);
		float _3144_3441(m1.data_[2][0] * m1.data_[3][3] - m1.data_[2][3] * m1.data_[3][0]);
		float _3243_3342(m1.data_[2][1] * m1.data_[3][2] - m1.data_[2][2] * m1.data_[3][1]);
		float _3244_3442(m1.data_[2][1] * m1.data_[3][3] - m1.data_[2][3] * m1.data_[3][1]);
		float _3344_3443(m1.data_[2][2] * m1.data_[3][3] - m1.data_[2][3] * m1.data_[3][2]);

		float det = m1.det();
		if (!equal<float>(det, 0))
		{
			float invDet=1.0f / det;

			mat44 tmp(
				+invDet * (m1.data_[1][1] * _3344_3443 - m1.data_[1][2] * _3244_3442 + m1.data_[1][3] * _3243_3342),
				-invDet * (m1.data_[0][1] * _3344_3443 - m1.data_[0][2] * _3244_3442 + m1.data_[0][3] * _3243_3342),
				+invDet * (m1.data_[0][1] * _2344_2443 - m1.data_[0][2] * _2244_2442 + m1.data_[0][3] * _2243_2342),
				-invDet * (m1.data_[0][1] * _2334_2433 - m1.data_[0][2] * _2234_2432 + m1.data_[0][3] * _2233_2332),

				-invDet * (m1.data_[1][0] * _3344_3443 - m1.data_[1][2] * _3144_3441 + m1.data_[1][3] * _3143_3341),
				+invDet * (m1.data_[0][0] * _3344_3443 - m1.data_[0][2] * _3144_3441 + m1.data_[0][3] * _3143_3341),
				-invDet * (m1.data_[0][0] * _2344_2443 - m1.data_[0][2] * _2144_2441 + m1.data_[0][3] * _2143_2341),
				+invDet * (m1.data_[0][0] * _2334_2433 - m1.data_[0][2] * _2134_2431 + m1.data_[0][3] * _2133_2331),

				+invDet * (m1.data_[1][0] * _3244_3442 - m1.data_[1][1] * _3144_3441 + m1.data_[1][3] * _3142_3241),
				-invDet * (m1.data_[0][0] * _3244_3442 - m1.data_[0][1] * _3144_3441 + m1.data_[0][3] * _3142_3241),
				+invDet * (m1.data_[0][0] * _2244_2442 - m1.data_[0][1] * _2144_2441 + m1.data_[0][3] * _2142_2241),
				-invDet * (m1.data_[0][0] * _2234_2432 - m1.data_[0][1] * _2134_2431 + m1.data_[0][3] * _2132_2231),

				-invDet * (m1.data_[1][0] * _3243_3342 - m1.data_[1][1] * _3143_3341 + m1.data_[1][2] * _3142_3241),
				+invDet * (m1.data_[0][0] * _3243_3342 - m1.data_[0][1] * _3143_3341 + m1.data_[0][2] * _3142_3241),
				-invDet * (m1.data_[0][0] * _2243_2342 - m1.data_[0][1] * _2143_2341 + m1.data_[0][2] * _2142_2241),
				+invDet * (m1.data_[0][0] * _2233_2332 - m1.data_[0][1] * _2133_2331 + m1.data_[0][2] * _2132_2231)
				);

			out = tmp;
		}

		return out;
	}

	mat44& mat_identity(mat44& out)
	{
		out = mat44::identity();
		return out;
	}

	mat44& mat_zero(mat44& out)
	{
		out = mat44::zero();
		return out;
	}

	/////////////////////////////////////////////
	//  matrix generator : for mathematics
	/////////////////////////////////////////////
	mat44& mat_rotate(mat44& out, const vec4& axis, float delta)
	{
		float s, c;
		sincos(radians(delta), s, c);

		out.data_[0][0]  = axis[0] * axis[0] + ( 1 - axis[0] * axis[0] ) * c;
		out.data_[1][0]  = axis[0] * axis[1] * ( 1 - c ) - axis[2] * s;
		out.data_[2][0]  = axis[0] * axis[2] * ( 1 - c ) + axis[1] * s;
		out.data_[3][0]  = 0;

		out.data_[0][1]  = axis[0] * axis[1] * ( 1 - c ) + axis[2] * s;
		out.data_[1][1]  = axis[1] * axis[1] + ( 1 - axis[1] * axis[1] ) * c;
		out.data_[2][1]  = axis[1] * axis[2] * ( 1 - c ) - axis[0] * s;
		out.data_[3][1]  = 0;

		out.data_[0][2]  = axis[0] * axis[2] * ( 1 - c ) - axis[1] * s;
		out.data_[1][2]  = axis[1] * axis[2] * ( 1 - c ) + axis[0] * s;
		out.data_[2][2]  = axis[2] * axis[2] + ( 1 - axis[2] * axis[2] ) * c;
		out.data_[3][2]  = 0;

		out.data_[0][3]  = 0;
		out.data_[1][3]  = 0;
		out.data_[2][3]  = 0;
		out.data_[3][3]  = 1;

		return out;
	}

	mat44& mat_rotX(mat44& out, float delta)
	{
		float s, c;
		sincos(radians(delta), s, c);
		mat_identity(out);

		out.data_[1][1] = c;
		out.data_[2][1] = -s;
		out.data_[1][2] = s;
		out.data_[2][2] = c;

		return out;
	}

	mat44& mat_rotY(mat44& out, float delta)
	{
		float s, c;
		sincos(radians(delta), s, c);
		mat_identity(out);

		out.data_[0][0] = c;
		out.data_[2][0] = s;
		out.data_[0][2] = -s;
		out.data_[2][2] = c;

		return out;
	}

	mat44& mat_rotZ(mat44& out, float delta)
	{
		float s, c;
		sincos(radians(delta), s, c);
		mat_identity(out);

		out.data_[0][0] = c;
		out.data_[1][0] = -s;
		out.data_[0][1] = s;
		out.data_[1][1] = c;

		return out;
	}

	mat44& mat_translate(mat44& out, float x, float y, float z)
	{
		mat_identity(out);
		out.set_row( 3, vec4(x, y, z, 1.0f) );
		return out;
	}

	mat44& mat_scale(mat44& out, float sx, float sy, float sz)
	{
		out = mat44::diag(sx, sy, sz, 1.0f);
		return out;
	}

	mat44& mat_reflect(mat44& out, const vec4& plane);

	mat44& mat_projection(mat44& out, float l, float r, float b, float t, float n, float f)
	{
		out = mat44(
			2.0f*n/(r-l),	0.0f,				0.0f,	0.0f,
			0.0f,				2.0f*n/(t-b),	0.0f,	0.0f,
			-(r+l)/(r-l),		-(t+b)/(t-b),	-f/(f-n),	1.0f,
			0.0f,				0.0f,			-n*f/(f-n),	0.0f
			);

		return out;
	}

	mat44& mat_perspective(mat44& out, float w, float h, float n, float f)
	{
		out = mat44(
			2*n/w,	0.0f,		0.0f,		0.0f,
			0.0f,		2*n/h,	0.0f,		0.0f,
			0.0f,		0.0f,		f/(f-n),	1.0f,
			0.0f,		0.0f,		-n*f/(f-n),	0.0f
			);
		return out;
	}

	mat44& mat_perspective_fov(mat44& out, float fovy, float aspect, float n, float f)
	{
		float ys = 1.0f / std::tan(fovy/2);
		float xs = ys / aspect;

		out = mat44(
			xs,		0.0f,	0.0f,		0.0f,
			0.0f,	ys,		0.0f,		0.0f,
			0.0f,	0.0f,	f/(f-n),		1.0f,
			0.0f,	0.0f,	-n*f/(f-n),	0.0f
			);

		return out;
	}

	mat44& mat_ortho(mat44& out, float l, float r, float b, float t, float n, float f)
	{
		out = mat44(
			2.0f/(r-l),		0.0f,			0.0f,			0.0f,
			0.0f,			2.0f/(t-b),		0.0f,			0.0f,
			0.0f,			0.0f,			1.0f/(f-n),		0.0f,
			(r+l)/(l-r),		(t+b)/(b-t),		n/(n-f),			1.0f
			);

		return out;
	}

	mat44& mat_lookat(mat44& out, const vec3& eye, const vec3& target, const vec3& up)
	{
		vec3 zdir = target - eye;
		zdir.normalize();
		vec3 xdir = cross_prod3(up.xyz(), zdir.xyz());
		xdir.normalize();
		vec3 ydir = cross_prod3(zdir.xyz(), xdir.xyz());

		out = mat44(
			xdir[0], ydir[0], zdir[0], 0.0f,
			xdir[1], ydir[1], zdir[1], 0.0f,
			xdir[2], ydir[2], zdir[2], 0.0f,
			-dot_prod3(xdir, eye), -dot_prod3(ydir, eye), -dot_prod3(zdir, eye), 1.0f);

		return out;
	}
}
