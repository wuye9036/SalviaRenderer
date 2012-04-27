#include <eflib/include/math/quaternion.h>
#include <eflib/include/math/math.h>
namespace eflib{

	quaternion::quaternion(){
	}

	quaternion::quaternion( float x, float y, float z, float w ): x(x), y(y), z(z), w(w){
	}

	quaternion::quaternion( const vec4& raw_v ):x(raw_v[0]), y(raw_v[1]), z(raw_v[2]), w(raw_v[3]){
	}

	quaternion quaternion::from_axis_angle( const vec3& axis, float angle ){
		vec3 normalized_axis = normalize3(axis);
		float half_angle = angle / 2.0f;
		float half_angle_sin = 0.0f;
		float half_angle_cos = 0.0f;

		sincos(half_angle, half_angle_sin, half_angle_cos);

		return quaternion(
			half_angle_sin * normalized_axis[0],
			half_angle_sin * normalized_axis[1],
			half_angle_sin * normalized_axis[2],
			half_angle_cos
			);
	}

	quaternion quaternion::from_mat44( const mat44& mat )
	{
		float _4_w_sqr_minus_1 = + mat.data_[0][0] + mat.data_[1][1] + mat.data_[2][2];
		float _4_x_sqr_minus_1 = + mat.data_[0][0] - mat.data_[1][1] - mat.data_[2][2];
		float _4_y_sqr_minus_1 = - mat.data_[0][0] + mat.data_[1][1] - mat.data_[2][2];
		float _4_z_sqr_minus_1 = - mat.data_[0][0] - mat.data_[1][1] + mat.data_[2][2];

		int i_biggest = 0;
		float biggest_val = _4_w_sqr_minus_1;

		if(_4_x_sqr_minus_1 > biggest_val){
			biggest_val = _4_x_sqr_minus_1;
			i_biggest = 1;
		}

		if(_4_y_sqr_minus_1 > biggest_val){
			biggest_val = _4_y_sqr_minus_1;
			i_biggest = 2;
		}

		if(_4_z_sqr_minus_1 > biggest_val){
			biggest_val = _4_z_sqr_minus_1;
			i_biggest = 3;
		}

		biggest_val = sqrt( biggest_val + 1.0f ) * 0.5f;
		float m = 0.25f / biggest_val;

		switch(i_biggest){
		case 0:
			return quaternion(
				(mat.data_[1][2] - mat.data_[2][1]) * m,
				(mat.data_[2][0] - mat.data_[0][2]) * m,
				(mat.data_[0][1] - mat.data_[1][0]) * m,
				biggest_val
				);
		case 1:
			return quaternion(
				biggest_val,
				(mat.data_[0][1] + mat.data_[1][0]) * m,
				(mat.data_[2][0] + mat.data_[0][2]) * m,
				(mat.data_[1][2] - mat.data_[2][1]) * m
				);
		case 2:
			return quaternion(
				(mat.data_[0][1] + mat.data_[1][0]) * m,
				biggest_val,
				(mat.data_[1][2] + mat.data_[2][1]) * m,
				(mat.data_[2][0] - mat.data_[0][2]) * m
				);
		default:
			return quaternion(
				(mat.data_[2][0] + mat.data_[0][2]) * m,
				(mat.data_[1][2] + mat.data_[2][1]) * m,
				biggest_val,
				(mat.data_[0][1] - mat.data_[1][0]) * m
				);

		}
	}

	quaternion quaternion::operator-() const
	{
		return quaternion(-x, -y, -z, -w);
	}

	quaternion& quaternion::operator*=( const quaternion& rhs )
	{
		*this = *this * rhs;
		return *this;
	}

	float quaternion::norm() const
	{
		return sqrt(x*x+y*y+z*z+w*w);
	}

	vec3 quaternion::axis() const
	{
		if( equal(w, 1.0f) ){
			return vec3();
		}
		return normalize3(vec3(x, y, z));
	}

	float quaternion::angle() const
	{
		return acos(w);
	}

	vec4 quaternion::comps() const
	{
		return vec4(x, y, z, w);
	}

	mat44 quaternion::to_mat44() const
	{
		return mat44(
			1.0f-2.0f*(y*y + z*z),	2.0f*(x*y-w*z),			2.0f*(w*y+x*z),			0.0f,
			2.0f*(x*y+w*z),			1.0f-2.0f*(x*x+z+z),	2.0f*(y*z-w*x),			0.0f,
			2.0f*(x*z-w*y),			2.0f*(y*z+w*x),			1.0f-2.0f*(x*x+y*y),	0.0f,
			0.0f,					0.0f,					0.0f,					1.0f
			);
	}

	quaternion normalize( const quaternion& lhs ){
		return lhs / lhs.norm();
	}

	quaternion conj( const quaternion& lhs ){
		return quaternion(-lhs.x, -lhs.y, -lhs.z, lhs.w);
	}

	quaternion inv( const quaternion& lhs ){
		return conj(lhs) / lhs.norm();
	}

	quaternion exp( const quaternion& lhs ){
		float alpha = lhs.norm();
		vec3 v = normalize3( lhs.comps().xyz() );

		float sin_alpha = 0.0f;
		float cos_alpha = 0.0f;
		sincos(alpha, sin_alpha, cos_alpha);

		vec4 q_v;
		q_v.w( cos_alpha );
		q_v.xyz( sin_alpha * v );

		return quaternion(q_v);
	}

	quaternion pow( const quaternion& lhs, float t ){
		if( !equal(lhs.w, 1.0f) ){
			float alpha = acos(lhs.w);
			float new_alpha = alpha * t;
			float w = cos(new_alpha);

			float mult = sin(new_alpha) / sin(alpha);
			return quaternion( lhs.x*mult, lhs.y*mult, lhs.z*mult, w);
		}
		return lhs;
	}

	quaternion log( const quaternion& lhs ){
		float alpha = acos(lhs.w);
		if( equal( std::abs(alpha), 1.0f ) ){
			return lhs;
		}
		vec3 new_v = normalize3( lhs.comps().xyz() );
		return quaternion(alpha*new_v[0], alpha*new_v[1], alpha*new_v[2], 0.0f);
	}

	quaternion operator*( const quaternion& lhs, const quaternion& rhs )
	{
		vec3 lhs_v(lhs.x, lhs.y, lhs.z);
		vec3 rhs_v(rhs.x, rhs.y, rhs.z);
		float w = lhs.w*rhs.w + dot_prod3(lhs_v, rhs_v);
		vec3 v = lhs.w*rhs_v + rhs.w*lhs_v + cross_prod3(rhs_v, lhs_v);
		return quaternion(v[0], v[1], v[2], w);
	}

	quaternion operator*( const quaternion& q, float scalar )
	{
		return quaternion( q.comps() * scalar );
	}

	quaternion operator*( float scalar, const quaternion& q )
	{
		return q * scalar;
	}

	quaternion operator/( const quaternion& lhs, const quaternion& rhs )
	{
		return lhs * inv(rhs);
	}

	quaternion operator/( const quaternion& q, float scalar )
	{
		return q * (1.0f / scalar);
	}

	vec3& transform( vec3& out, const quaternion& q, const vec3& v )
	{
		quaternion vq(v[0], v[1], v[2], 0.0f);
		out = ( q * vq * conj(q) ).comps().xyz();
		return out;
	}

	quaternion slerp( const quaternion& src, const quaternion& dest, float t )
	{
		float cos_omega = dot_prod4(src.comps(), dest.comps());
		vec4 near_dest_v = dest.comps() * sign(cos_omega);
		cos_omega = abs(cos_omega);

		float k0(0.0f), k1(0.0f);
		if( equal(cos_omega, 1.0f) ){
			k0 = 1.0f - t;
			k1 = t;
		}else{
			float sin_omega = sqrt(1.0f - cos_omega*cos_omega);
			float omega = atan2(sin_omega, cos_omega);
			float inv_sin_omega = 1.0f / sin_omega;
			k0 = sin( (1.0f - t) * omega ) * inv_sin_omega;
			k1 = sin( t * omega ) * inv_sin_omega;
		}

		return quaternion( src.comps() * k0 + near_dest_v * k1);
	}

}
