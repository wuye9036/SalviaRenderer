#ifndef EFLIB_MATH_QUATERNION_H
#define EFLIB_MATH_QUATERNION_H

#include "vector.h"
#include "matrix.h"

namespace eflib{

class eular_angle{
private:
	float yaw, pitch, row;
};

class quaternion
{
private:
	float x, y, z, w;
public:
	// Constructors
	quaternion();
	quaternion(float x, float y, float z, float w);
	quaternion(const vec4& raw_v);

	static quaternion from_eular(const eular_angle& ea);
	static quaternion from_axis_angle(const vec3& axis, float angle);

	static quaternion from_mat44(const mat44& mat);
	static quaternion from_2_vecs(const vec3& from, const vec3& to);

	//negative operator
	quaternion operator - () const;

	quaternion& operator *= (const quaternion& rhs);

	float norm() const;

	vec3 axis() const;
	float angle() const;

	vec4 comps() const;

	mat44 to_mat44() const;

	void normalize();

	//friend operators
	friend quaternion normalize(const quaternion& lhs);

	friend quaternion conj(const quaternion& lhs);
	friend quaternion inv(const quaternion& lhs);

	friend quaternion exp(const quaternion& lhs);
	friend quaternion pow(const quaternion& lhs, float t);
	friend quaternion log(const quaternion& lhs);

	friend quaternion operator * (const quaternion& lhs, const quaternion& rhs);
	friend quaternion operator * (const quaternion& q, float scalar);
	friend quaternion operator * (float scalar, const quaternion& q);

	friend quaternion operator / (const quaternion& lhs, const quaternion& rhs);
	friend quaternion operator / (const quaternion& q, float scalar);

	friend vec3& transform(vec3& out, const quaternion& q, const vec3& v);

	friend quaternion lerp(const quaternion& src, const quaternion& dest, float t);
	friend quaternion slerp(const quaternion& src, const quaternion& dest, float t);
};

}//namespace
#endif