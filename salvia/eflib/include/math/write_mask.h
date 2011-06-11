#ifndef EFLIB_WRITE_MASK_H
#define EFLIB_WRITE_MASK_H


#define WRITE_MASK_FOR_VEC2() \
void xy(float x,float y){\
	this->x = x;\
	this->y = y;\
}\
void xy(const vec2& v){\
	x = v.x;\
	y = v.y;\
}\
void yx(float y,float x){\
	this->y = y;\
	this->x = x;\
}\
void yx(const vec2& v){\
	y = v.x;\
	x = v.y;\
}\


#define WRITE_MASK_FOR_VEC3() \
void xy(float x,float y){\
	this->x = x;\
	this->y = y;\
}\
void xy(const vec2& v){\
	x = v.x;\
	y = v.y;\
}\
void yx(float y,float x){\
	this->y = y;\
	this->x = x;\
}\
void yx(const vec2& v){\
	y = v.x;\
	x = v.y;\
}\
void xz(float x,float z){\
	this->x = x;\
	this->z = z;\
}\
void xz(const vec2& v){\
	x = v.x;\
	z = v.y;\
}\
void zx(float z,float x){\
	this->z = z;\
	this->x = x;\
}\
void zx(const vec2& v){\
	z = v.x;\
	x = v.y;\
}\
void yz(float y,float z){\
	this->y = y;\
	this->z = z;\
}\
void yz(const vec2& v){\
	y = v.x;\
	z = v.y;\
}\
void zy(float z,float y){\
	this->z = z;\
	this->y = y;\
}\
void zy(const vec2& v){\
	z = v.x;\
	y = v.y;\
}\
void xyz(float x,float y,float z){\
	this->x = x;\
	this->y = y;\
	this->z = z;\
}\
void xyz(const vec3& v){\
	x = v.x;\
	y = v.y;\
	z = v.z;\
}\
void xzy(float x,float z,float y){\
	this->x = x;\
	this->z = z;\
	this->y = y;\
}\
void xzy(const vec3& v){\
	x = v.x;\
	z = v.y;\
	y = v.z;\
}\
void yxz(float y,float x,float z){\
	this->y = y;\
	this->x = x;\
	this->z = z;\
}\
void yxz(const vec3& v){\
	y = v.x;\
	x = v.y;\
	z = v.z;\
}\
void yzx(float y,float z,float x){\
	this->y = y;\
	this->z = z;\
	this->x = x;\
}\
void yzx(const vec3& v){\
	y = v.x;\
	z = v.y;\
	x = v.z;\
}\
void zxy(float z,float x,float y){\
	this->z = z;\
	this->x = x;\
	this->y = y;\
}\
void zxy(const vec3& v){\
	z = v.x;\
	x = v.y;\
	y = v.z;\
}\
void zyx(float z,float y,float x){\
	this->z = z;\
	this->y = y;\
	this->x = x;\
}\
void zyx(const vec3& v){\
	z = v.x;\
	y = v.y;\
	x = v.z;\
}\


#define WRITE_MASK_FOR_VEC4() \
void xy(float x,float y){\
	this->x = x;\
	this->y = y;\
}\
void xy(const vec2& v){\
	x = v.x;\
	y = v.y;\
}\
void yx(float y,float x){\
	this->y = y;\
	this->x = x;\
}\
void yx(const vec2& v){\
	y = v.x;\
	x = v.y;\
}\
void xz(float x,float z){\
	this->x = x;\
	this->z = z;\
}\
void xz(const vec2& v){\
	x = v.x;\
	z = v.y;\
}\
void zx(float z,float x){\
	this->z = z;\
	this->x = x;\
}\
void zx(const vec2& v){\
	z = v.x;\
	x = v.y;\
}\
void xw(float x,float w){\
	this->x = x;\
	this->w = w;\
}\
void xw(const vec2& v){\
	x = v.x;\
	w = v.y;\
}\
void wx(float w,float x){\
	this->w = w;\
	this->x = x;\
}\
void wx(const vec2& v){\
	w = v.x;\
	x = v.y;\
}\
void yz(float y,float z){\
	this->y = y;\
	this->z = z;\
}\
void yz(const vec2& v){\
	y = v.x;\
	z = v.y;\
}\
void zy(float z,float y){\
	this->z = z;\
	this->y = y;\
}\
void zy(const vec2& v){\
	z = v.x;\
	y = v.y;\
}\
void yw(float y,float w){\
	this->y = y;\
	this->w = w;\
}\
void yw(const vec2& v){\
	y = v.x;\
	w = v.y;\
}\
void wy(float w,float y){\
	this->w = w;\
	this->y = y;\
}\
void wy(const vec2& v){\
	w = v.x;\
	y = v.y;\
}\
void zw(float z,float w){\
	this->z = z;\
	this->w = w;\
}\
void zw(const vec2& v){\
	z = v.x;\
	w = v.y;\
}\
void wz(float w,float z){\
	this->w = w;\
	this->z = z;\
}\
void wz(const vec2& v){\
	w = v.x;\
	z = v.y;\
}\
void xyz(float x,float y,float z){\
	this->x = x;\
	this->y = y;\
	this->z = z;\
}\
void xyz(const vec3& v){\
	x = v.x;\
	y = v.y;\
	z = v.z;\
}\
void xzy(float x,float z,float y){\
	this->x = x;\
	this->z = z;\
	this->y = y;\
}\
void xzy(const vec3& v){\
	x = v.x;\
	z = v.y;\
	y = v.z;\
}\
void yxz(float y,float x,float z){\
	this->y = y;\
	this->x = x;\
	this->z = z;\
}\
void yxz(const vec3& v){\
	y = v.x;\
	x = v.y;\
	z = v.z;\
}\
void yzx(float y,float z,float x){\
	this->y = y;\
	this->z = z;\
	this->x = x;\
}\
void yzx(const vec3& v){\
	y = v.x;\
	z = v.y;\
	x = v.z;\
}\
void zxy(float z,float x,float y){\
	this->z = z;\
	this->x = x;\
	this->y = y;\
}\
void zxy(const vec3& v){\
	z = v.x;\
	x = v.y;\
	y = v.z;\
}\
void zyx(float z,float y,float x){\
	this->z = z;\
	this->y = y;\
	this->x = x;\
}\
void zyx(const vec3& v){\
	z = v.x;\
	y = v.y;\
	x = v.z;\
}\
void xyw(float x,float y,float w){\
	this->x = x;\
	this->y = y;\
	this->w = w;\
}\
void xyw(const vec3& v){\
	x = v.x;\
	y = v.y;\
	w = v.z;\
}\
void xwy(float x,float w,float y){\
	this->x = x;\
	this->w = w;\
	this->y = y;\
}\
void xwy(const vec3& v){\
	x = v.x;\
	w = v.y;\
	y = v.z;\
}\
void yxw(float y,float x,float w){\
	this->y = y;\
	this->x = x;\
	this->w = w;\
}\
void yxw(const vec3& v){\
	y = v.x;\
	x = v.y;\
	w = v.z;\
}\
void ywx(float y,float w,float x){\
	this->y = y;\
	this->w = w;\
	this->x = x;\
}\
void ywx(const vec3& v){\
	y = v.x;\
	w = v.y;\
	x = v.z;\
}\
void wxy(float w,float x,float y){\
	this->w = w;\
	this->x = x;\
	this->y = y;\
}\
void wxy(const vec3& v){\
	w = v.x;\
	x = v.y;\
	y = v.z;\
}\
void wyx(float w,float y,float x){\
	this->w = w;\
	this->y = y;\
	this->x = x;\
}\
void wyx(const vec3& v){\
	w = v.x;\
	y = v.y;\
	x = v.z;\
}\
void xzw(float x,float z,float w){\
	this->x = x;\
	this->z = z;\
	this->w = w;\
}\
void xzw(const vec3& v){\
	x = v.x;\
	z = v.y;\
	w = v.z;\
}\
void xwz(float x,float w,float z){\
	this->x = x;\
	this->w = w;\
	this->z = z;\
}\
void xwz(const vec3& v){\
	x = v.x;\
	w = v.y;\
	z = v.z;\
}\
void zxw(float z,float x,float w){\
	this->z = z;\
	this->x = x;\
	this->w = w;\
}\
void zxw(const vec3& v){\
	z = v.x;\
	x = v.y;\
	w = v.z;\
}\
void zwx(float z,float w,float x){\
	this->z = z;\
	this->w = w;\
	this->x = x;\
}\
void zwx(const vec3& v){\
	z = v.x;\
	w = v.y;\
	x = v.z;\
}\
void wxz(float w,float x,float z){\
	this->w = w;\
	this->x = x;\
	this->z = z;\
}\
void wxz(const vec3& v){\
	w = v.x;\
	x = v.y;\
	z = v.z;\
}\
void wzx(float w,float z,float x){\
	this->w = w;\
	this->z = z;\
	this->x = x;\
}\
void wzx(const vec3& v){\
	w = v.x;\
	z = v.y;\
	x = v.z;\
}\
void yzw(float y,float z,float w){\
	this->y = y;\
	this->z = z;\
	this->w = w;\
}\
void yzw(const vec3& v){\
	y = v.x;\
	z = v.y;\
	w = v.z;\
}\
void ywz(float y,float w,float z){\
	this->y = y;\
	this->w = w;\
	this->z = z;\
}\
void ywz(const vec3& v){\
	y = v.x;\
	w = v.y;\
	z = v.z;\
}\
void zyw(float z,float y,float w){\
	this->z = z;\
	this->y = y;\
	this->w = w;\
}\
void zyw(const vec3& v){\
	z = v.x;\
	y = v.y;\
	w = v.z;\
}\
void zwy(float z,float w,float y){\
	this->z = z;\
	this->w = w;\
	this->y = y;\
}\
void zwy(const vec3& v){\
	z = v.x;\
	w = v.y;\
	y = v.z;\
}\
void wyz(float w,float y,float z){\
	this->w = w;\
	this->y = y;\
	this->z = z;\
}\
void wyz(const vec3& v){\
	w = v.x;\
	y = v.y;\
	z = v.z;\
}\
void wzy(float w,float z,float y){\
	this->w = w;\
	this->z = z;\
	this->y = y;\
}\
void wzy(const vec3& v){\
	w = v.x;\
	z = v.y;\
	y = v.z;\
}\
void xyzw(float x,float y,float z,float w){\
	this->x = x;\
	this->y = y;\
	this->z = z;\
	this->w = w;\
}\
void xyzw(const vec4& v){\
	x = v.x;\
	y = v.y;\
	z = v.z;\
	w = v.w;\
}\
void xywz(float x,float y,float w,float z){\
	this->x = x;\
	this->y = y;\
	this->w = w;\
	this->z = z;\
}\
void xywz(const vec4& v){\
	x = v.x;\
	y = v.y;\
	w = v.z;\
	z = v.w;\
}\
void xzyw(float x,float z,float y,float w){\
	this->x = x;\
	this->z = z;\
	this->y = y;\
	this->w = w;\
}\
void xzyw(const vec4& v){\
	x = v.x;\
	z = v.y;\
	y = v.z;\
	w = v.w;\
}\
void xzwy(float x,float z,float w,float y){\
	this->x = x;\
	this->z = z;\
	this->w = w;\
	this->y = y;\
}\
void xzwy(const vec4& v){\
	x = v.x;\
	z = v.y;\
	w = v.z;\
	y = v.w;\
}\
void xwyz(float x,float w,float y,float z){\
	this->x = x;\
	this->w = w;\
	this->y = y;\
	this->z = z;\
}\
void xwyz(const vec4& v){\
	x = v.x;\
	w = v.y;\
	y = v.z;\
	z = v.w;\
}\
void xwzy(float x,float w,float z,float y){\
	this->x = x;\
	this->w = w;\
	this->z = z;\
	this->y = y;\
}\
void xwzy(const vec4& v){\
	x = v.x;\
	w = v.y;\
	z = v.z;\
	y = v.w;\
}\
void yxzw(float y,float x,float z,float w){\
	this->y = y;\
	this->x = x;\
	this->z = z;\
	this->w = w;\
}\
void yxzw(const vec4& v){\
	y = v.x;\
	x = v.y;\
	z = v.z;\
	w = v.w;\
}\
void yxwz(float y,float x,float w,float z){\
	this->y = y;\
	this->x = x;\
	this->w = w;\
	this->z = z;\
}\
void yxwz(const vec4& v){\
	y = v.x;\
	x = v.y;\
	w = v.z;\
	z = v.w;\
}\
void yzxw(float y,float z,float x,float w){\
	this->y = y;\
	this->z = z;\
	this->x = x;\
	this->w = w;\
}\
void yzxw(const vec4& v){\
	y = v.x;\
	z = v.y;\
	x = v.z;\
	w = v.w;\
}\
void yzwx(float y,float z,float w,float x){\
	this->y = y;\
	this->z = z;\
	this->w = w;\
	this->x = x;\
}\
void yzwx(const vec4& v){\
	y = v.x;\
	z = v.y;\
	w = v.z;\
	x = v.w;\
}\
void ywxz(float y,float w,float x,float z){\
	this->y = y;\
	this->w = w;\
	this->x = x;\
	this->z = z;\
}\
void ywxz(const vec4& v){\
	y = v.x;\
	w = v.y;\
	x = v.z;\
	z = v.w;\
}\
void ywzx(float y,float w,float z,float x){\
	this->y = y;\
	this->w = w;\
	this->z = z;\
	this->x = x;\
}\
void ywzx(const vec4& v){\
	y = v.x;\
	w = v.y;\
	z = v.z;\
	x = v.w;\
}\
void zxyw(float z,float x,float y,float w){\
	this->z = z;\
	this->x = x;\
	this->y = y;\
	this->w = w;\
}\
void zxyw(const vec4& v){\
	z = v.x;\
	x = v.y;\
	y = v.z;\
	w = v.w;\
}\
void zxwy(float z,float x,float w,float y){\
	this->z = z;\
	this->x = x;\
	this->w = w;\
	this->y = y;\
}\
void zxwy(const vec4& v){\
	z = v.x;\
	x = v.y;\
	w = v.z;\
	y = v.w;\
}\
void zyxw(float z,float y,float x,float w){\
	this->z = z;\
	this->y = y;\
	this->x = x;\
	this->w = w;\
}\
void zyxw(const vec4& v){\
	z = v.x;\
	y = v.y;\
	x = v.z;\
	w = v.w;\
}\
void zywx(float z,float y,float w,float x){\
	this->z = z;\
	this->y = y;\
	this->w = w;\
	this->x = x;\
}\
void zywx(const vec4& v){\
	z = v.x;\
	y = v.y;\
	w = v.z;\
	x = v.w;\
}\
void zwxy(float z,float w,float x,float y){\
	this->z = z;\
	this->w = w;\
	this->x = x;\
	this->y = y;\
}\
void zwxy(const vec4& v){\
	z = v.x;\
	w = v.y;\
	x = v.z;\
	y = v.w;\
}\
void zwyx(float z,float w,float y,float x){\
	this->z = z;\
	this->w = w;\
	this->y = y;\
	this->x = x;\
}\
void zwyx(const vec4& v){\
	z = v.x;\
	w = v.y;\
	y = v.z;\
	x = v.w;\
}\
void wxyz(float w,float x,float y,float z){\
	this->w = w;\
	this->x = x;\
	this->y = y;\
	this->z = z;\
}\
void wxyz(const vec4& v){\
	w = v.x;\
	x = v.y;\
	y = v.z;\
	z = v.w;\
}\
void wxzy(float w,float x,float z,float y){\
	this->w = w;\
	this->x = x;\
	this->z = z;\
	this->y = y;\
}\
void wxzy(const vec4& v){\
	w = v.x;\
	x = v.y;\
	z = v.z;\
	y = v.w;\
}\
void wyxz(float w,float y,float x,float z){\
	this->w = w;\
	this->y = y;\
	this->x = x;\
	this->z = z;\
}\
void wyxz(const vec4& v){\
	w = v.x;\
	y = v.y;\
	x = v.z;\
	z = v.w;\
}\
void wyzx(float w,float y,float z,float x){\
	this->w = w;\
	this->y = y;\
	this->z = z;\
	this->x = x;\
}\
void wyzx(const vec4& v){\
	w = v.x;\
	y = v.y;\
	z = v.z;\
	x = v.w;\
}\
void wzxy(float w,float z,float x,float y){\
	this->w = w;\
	this->z = z;\
	this->x = x;\
	this->y = y;\
}\
void wzxy(const vec4& v){\
	w = v.x;\
	z = v.y;\
	x = v.z;\
	y = v.w;\
}\
void wzyx(float w,float z,float y,float x){\
	this->w = w;\
	this->z = z;\
	this->y = y;\
	this->x = x;\
}\
void wzyx(const vec4& v){\
	w = v.x;\
	z = v.y;\
	y = v.z;\
	x = v.w;\
}\

#endif