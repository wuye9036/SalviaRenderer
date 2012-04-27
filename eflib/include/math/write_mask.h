#ifndef EFLIB_WRITE_MASK_H
#define EFLIB_WRITE_MASK_H


#define WRITE_MASK_FOR_VEC2() \
void x(ScalarT x){\
	((ScalarT*)this)[0] = x;\
}\
void y(ScalarT y){\
	((ScalarT*)this)[1] = y;\
}\
void xy(ScalarT x, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void xy(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
}\
void yx(ScalarT y, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void yx(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
}\


#define WRITE_MASK_FOR_VEC3() \
void x(ScalarT x){\
	((ScalarT*)this)[0] = x;\
}\
void y(ScalarT y){\
	((ScalarT*)this)[1] = y;\
}\
void z(ScalarT z){\
	((ScalarT*)this)[2] = z;\
}\
void xy(ScalarT x, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void xy(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
}\
void yx(ScalarT y, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void yx(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
}\
void xz(ScalarT x, ScalarT z){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
}\
void xz(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[2] = v[1];\
}\
void zx(ScalarT z, ScalarT x){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
}\
void zx(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[0] = v[1];\
}\
void yz(ScalarT y, ScalarT z){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
}\
void yz(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[2] = v[1];\
}\
void zy(ScalarT z, ScalarT y){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
}\
void zy(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[1] = v[1];\
}\
void xyz(ScalarT x, ScalarT y, ScalarT z){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
}\
void xyz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void xzy(ScalarT x, ScalarT z, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
}\
void xzy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void yxz(ScalarT y, ScalarT x, ScalarT z){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
}\
void yxz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void yzx(ScalarT y, ScalarT z, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
}\
void yzx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\
void zxy(ScalarT z, ScalarT x, ScalarT y){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void zxy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void zyx(ScalarT z, ScalarT y, ScalarT x){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void zyx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\


#define WRITE_MASK_FOR_VEC4() \
void x(ScalarT x){\
	((ScalarT*)this)[0] = x;\
}\
void y(ScalarT y){\
	((ScalarT*)this)[1] = y;\
}\
void z(ScalarT z){\
	((ScalarT*)this)[2] = z;\
}\
void w(ScalarT w){\
	((ScalarT*)this)[3] = w;\
}\
void xy(ScalarT x, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void xy(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
}\
void yx(ScalarT y, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void yx(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
}\
void xz(ScalarT x, ScalarT z){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
}\
void xz(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[2] = v[1];\
}\
void zx(ScalarT z, ScalarT x){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
}\
void zx(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[0] = v[1];\
}\
void xw(ScalarT x, ScalarT w){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
}\
void xw(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[3] = v[1];\
}\
void wx(ScalarT w, ScalarT x){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
}\
void wx(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[0] = v[1];\
}\
void yz(ScalarT y, ScalarT z){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
}\
void yz(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[2] = v[1];\
}\
void zy(ScalarT z, ScalarT y){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
}\
void zy(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[1] = v[1];\
}\
void yw(ScalarT y, ScalarT w){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
}\
void yw(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[3] = v[1];\
}\
void wy(ScalarT w, ScalarT y){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
}\
void wy(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[1] = v[1];\
}\
void zw(ScalarT z, ScalarT w){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
}\
void zw(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[3] = v[1];\
}\
void wz(ScalarT w, ScalarT z){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
}\
void wz(vector_<ScalarT,2> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[2] = v[1];\
}\
void xyz(ScalarT x, ScalarT y, ScalarT z){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
}\
void xyz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void xzy(ScalarT x, ScalarT z, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
}\
void xzy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void yxz(ScalarT y, ScalarT x, ScalarT z){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
}\
void yxz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void yzx(ScalarT y, ScalarT z, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
}\
void yzx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\
void zxy(ScalarT z, ScalarT x, ScalarT y){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void zxy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void zyx(ScalarT z, ScalarT y, ScalarT x){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void zyx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\
void xyw(ScalarT x, ScalarT y, ScalarT w){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
}\
void xyw(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[3] = v[2];\
}\
void xwy(ScalarT x, ScalarT w, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
}\
void xwy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void yxw(ScalarT y, ScalarT x, ScalarT w){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
}\
void yxw(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[3] = v[2];\
}\
void ywx(ScalarT y, ScalarT w, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
}\
void ywx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\
void wxy(ScalarT w, ScalarT x, ScalarT y){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void wxy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void wyx(ScalarT w, ScalarT y, ScalarT x){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void wyx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\
void xzw(ScalarT x, ScalarT z, ScalarT w){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
}\
void xzw(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[3] = v[2];\
}\
void xwz(ScalarT x, ScalarT w, ScalarT z){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
}\
void xwz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void zxw(ScalarT z, ScalarT x, ScalarT w){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
}\
void zxw(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[3] = v[2];\
}\
void zwx(ScalarT z, ScalarT w, ScalarT x){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
}\
void zwx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\
void wxz(ScalarT w, ScalarT x, ScalarT z){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
}\
void wxz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void wzx(ScalarT w, ScalarT z, ScalarT x){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
}\
void wzx(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[0] = v[2];\
}\
void yzw(ScalarT y, ScalarT z, ScalarT w){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
}\
void yzw(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[3] = v[2];\
}\
void ywz(ScalarT y, ScalarT w, ScalarT z){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
}\
void ywz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void zyw(ScalarT z, ScalarT y, ScalarT w){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
}\
void zyw(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[3] = v[2];\
}\
void zwy(ScalarT z, ScalarT w, ScalarT y){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
}\
void zwy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void wyz(ScalarT w, ScalarT y, ScalarT z){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
}\
void wyz(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[2] = v[2];\
}\
void wzy(ScalarT w, ScalarT z, ScalarT y){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
}\
void wzy(vector_<ScalarT,3> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[1] = v[2];\
}\
void xyzw(ScalarT x, ScalarT y, ScalarT z, ScalarT w){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
}\
void xyzw(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[2] = v[2];\
	((ScalarT*)this)[3] = v[3];\
}\
void xywz(ScalarT x, ScalarT y, ScalarT w, ScalarT z){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
}\
void xywz(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[3] = v[2];\
	((ScalarT*)this)[2] = v[3];\
}\
void xzyw(ScalarT x, ScalarT z, ScalarT y, ScalarT w){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
}\
void xzyw(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[1] = v[2];\
	((ScalarT*)this)[3] = v[3];\
}\
void xzwy(ScalarT x, ScalarT z, ScalarT w, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
}\
void xzwy(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[3] = v[2];\
	((ScalarT*)this)[1] = v[3];\
}\
void xwyz(ScalarT x, ScalarT w, ScalarT y, ScalarT z){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
}\
void xwyz(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[1] = v[2];\
	((ScalarT*)this)[2] = v[3];\
}\
void xwzy(ScalarT x, ScalarT w, ScalarT z, ScalarT y){\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
}\
void xwzy(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[0] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[2] = v[2];\
	((ScalarT*)this)[1] = v[3];\
}\
void yxzw(ScalarT y, ScalarT x, ScalarT z, ScalarT w){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
}\
void yxzw(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[2] = v[2];\
	((ScalarT*)this)[3] = v[3];\
}\
void yxwz(ScalarT y, ScalarT x, ScalarT w, ScalarT z){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
}\
void yxwz(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[3] = v[2];\
	((ScalarT*)this)[2] = v[3];\
}\
void yzxw(ScalarT y, ScalarT z, ScalarT x, ScalarT w){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
}\
void yzxw(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[0] = v[2];\
	((ScalarT*)this)[3] = v[3];\
}\
void yzwx(ScalarT y, ScalarT z, ScalarT w, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
}\
void yzwx(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[3] = v[2];\
	((ScalarT*)this)[0] = v[3];\
}\
void ywxz(ScalarT y, ScalarT w, ScalarT x, ScalarT z){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
}\
void ywxz(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[0] = v[2];\
	((ScalarT*)this)[2] = v[3];\
}\
void ywzx(ScalarT y, ScalarT w, ScalarT z, ScalarT x){\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
}\
void ywzx(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[1] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[2] = v[2];\
	((ScalarT*)this)[0] = v[3];\
}\
void zxyw(ScalarT z, ScalarT x, ScalarT y, ScalarT w){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
}\
void zxyw(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[1] = v[2];\
	((ScalarT*)this)[3] = v[3];\
}\
void zxwy(ScalarT z, ScalarT x, ScalarT w, ScalarT y){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
}\
void zxwy(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[3] = v[2];\
	((ScalarT*)this)[1] = v[3];\
}\
void zyxw(ScalarT z, ScalarT y, ScalarT x, ScalarT w){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[3] = w;\
}\
void zyxw(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[0] = v[2];\
	((ScalarT*)this)[3] = v[3];\
}\
void zywx(ScalarT z, ScalarT y, ScalarT w, ScalarT x){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
}\
void zywx(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[3] = v[2];\
	((ScalarT*)this)[0] = v[3];\
}\
void zwxy(ScalarT z, ScalarT w, ScalarT x, ScalarT y){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void zwxy(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[0] = v[2];\
	((ScalarT*)this)[1] = v[3];\
}\
void zwyx(ScalarT z, ScalarT w, ScalarT y, ScalarT x){\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void zwyx(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[2] = v[0];\
	((ScalarT*)this)[3] = v[1];\
	((ScalarT*)this)[1] = v[2];\
	((ScalarT*)this)[0] = v[3];\
}\
void wxyz(ScalarT w, ScalarT x, ScalarT y, ScalarT z){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
}\
void wxyz(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[1] = v[2];\
	((ScalarT*)this)[2] = v[3];\
}\
void wxzy(ScalarT w, ScalarT x, ScalarT z, ScalarT y){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
}\
void wxzy(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[0] = v[1];\
	((ScalarT*)this)[2] = v[2];\
	((ScalarT*)this)[1] = v[3];\
}\
void wyxz(ScalarT w, ScalarT y, ScalarT x, ScalarT z){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[2] = z;\
}\
void wyxz(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[0] = v[2];\
	((ScalarT*)this)[2] = v[3];\
}\
void wyzx(ScalarT w, ScalarT y, ScalarT z, ScalarT x){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
}\
void wyzx(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[1] = v[1];\
	((ScalarT*)this)[2] = v[2];\
	((ScalarT*)this)[0] = v[3];\
}\
void wzxy(ScalarT w, ScalarT z, ScalarT x, ScalarT y){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[0] = x;\
	((ScalarT*)this)[1] = y;\
}\
void wzxy(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[0] = v[2];\
	((ScalarT*)this)[1] = v[3];\
}\
void wzyx(ScalarT w, ScalarT z, ScalarT y, ScalarT x){\
	((ScalarT*)this)[3] = w;\
	((ScalarT*)this)[2] = z;\
	((ScalarT*)this)[1] = y;\
	((ScalarT*)this)[0] = x;\
}\
void wzyx(vector_<ScalarT,4> const& v){\
	((ScalarT*)this)[3] = v[0];\
	((ScalarT*)this)[2] = v[1];\
	((ScalarT*)this)[1] = v[2];\
	((ScalarT*)this)[0] = v[3];\
}\

#endif