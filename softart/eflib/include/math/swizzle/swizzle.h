#ifndef EFLIB_SWIZZLE_H
#define EFLIB_SWIZZLE_H

#define SWIZZLE_DECL_FOR_VEC2() \
	vec2 xx() const;\
	vec3 xxx() const;\
	vec4 xxxx() const;\
	vec2 xy() const;\
	vec2 yx() const;\
	vec2 yy() const;\
	vec3 xxy() const;\
	vec3 xyx() const;\
	vec3 xyy() const;\
	vec3 yxx() const;\
	vec3 yxy() const;\
	vec3 yyx() const;\
	vec3 yyy() const;\
	vec4 xxxy() const;\
	vec4 xxyx() const;\
	vec4 xxyy() const;\
	vec4 xyxx() const;\
	vec4 xyxy() const;\
	vec4 xyyx() const;\
	vec4 xyyy() const;\
	vec4 yxxx() const;\
	vec4 yxxy() const;\
	vec4 yxyx() const;\
	vec4 yxyy() const;\
	vec4 yyxx() const;\
	vec4 yyxy() const;\
	vec4 yyyx() const;\
	vec4 yyyy() const;\
	;

#define SWIZZLE_DECL_FOR_VEC3() \
	vec2 xx() const;\
	vec3 xxx() const;\
	vec4 xxxx() const;\
	vec2 xy() const;\
	vec2 yx() const;\
	vec2 yy() const;\
	vec3 xxy() const;\
	vec3 xyx() const;\
	vec3 xyy() const;\
	vec3 yxx() const;\
	vec3 yxy() const;\
	vec3 yyx() const;\
	vec3 yyy() const;\
	vec4 xxxy() const;\
	vec4 xxyx() const;\
	vec4 xxyy() const;\
	vec4 xyxx() const;\
	vec4 xyxy() const;\
	vec4 xyyx() const;\
	vec4 xyyy() const;\
	vec4 yxxx() const;\
	vec4 yxxy() const;\
	vec4 yxyx() const;\
	vec4 yxyy() const;\
	vec4 yyxx() const;\
	vec4 yyxy() const;\
	vec4 yyyx() const;\
	vec4 yyyy() const;\
	vec2 xz() const;\
	vec2 yz() const;\
	vec2 zx() const;\
	vec2 zy() const;\
	vec2 zz() const;\
	vec3 xxz() const;\
	vec3 xyz() const;\
	vec3 xzx() const;\
	vec3 xzy() const;\
	vec3 xzz() const;\
	vec3 yxz() const;\
	vec3 yyz() const;\
	vec3 yzx() const;\
	vec3 yzy() const;\
	vec3 yzz() const;\
	vec3 zxx() const;\
	vec3 zxy() const;\
	vec3 zxz() const;\
	vec3 zyx() const;\
	vec3 zyy() const;\
	vec3 zyz() const;\
	vec3 zzx() const;\
	vec3 zzy() const;\
	vec3 zzz() const;\
	vec4 xxxz() const;\
	vec4 xxyz() const;\
	vec4 xxzx() const;\
	vec4 xxzy() const;\
	vec4 xxzz() const;\
	vec4 xyxz() const;\
	vec4 xyyz() const;\
	vec4 xyzx() const;\
	vec4 xyzy() const;\
	vec4 xyzz() const;\
	vec4 xzxx() const;\
	vec4 xzxy() const;\
	vec4 xzxz() const;\
	vec4 xzyx() const;\
	vec4 xzyy() const;\
	vec4 xzyz() const;\
	vec4 xzzx() const;\
	vec4 xzzy() const;\
	vec4 xzzz() const;\
	vec4 yxxz() const;\
	vec4 yxyz() const;\
	vec4 yxzx() const;\
	vec4 yxzy() const;\
	vec4 yxzz() const;\
	vec4 yyxz() const;\
	vec4 yyyz() const;\
	vec4 yyzx() const;\
	vec4 yyzy() const;\
	vec4 yyzz() const;\
	vec4 yzxx() const;\
	vec4 yzxy() const;\
	vec4 yzxz() const;\
	vec4 yzyx() const;\
	vec4 yzyy() const;\
	vec4 yzyz() const;\
	vec4 yzzx() const;\
	vec4 yzzy() const;\
	vec4 yzzz() const;\
	vec4 zxxx() const;\
	vec4 zxxy() const;\
	vec4 zxxz() const;\
	vec4 zxyx() const;\
	vec4 zxyy() const;\
	vec4 zxyz() const;\
	vec4 zxzx() const;\
	vec4 zxzy() const;\
	vec4 zxzz() const;\
	vec4 zyxx() const;\
	vec4 zyxy() const;\
	vec4 zyxz() const;\
	vec4 zyyx() const;\
	vec4 zyyy() const;\
	vec4 zyyz() const;\
	vec4 zyzx() const;\
	vec4 zyzy() const;\
	vec4 zyzz() const;\
	vec4 zzxx() const;\
	vec4 zzxy() const;\
	vec4 zzxz() const;\
	vec4 zzyx() const;\
	vec4 zzyy() const;\
	vec4 zzyz() const;\
	vec4 zzzx() const;\
	vec4 zzzy() const;\
	vec4 zzzz() const;\
	;

#define SWIZZLE_DECL_FOR_VEC4() \
	vec2 xx() const;\
	vec3 xxx() const;\
	vec4 xxxx() const;\
	vec2 xy() const;\
	vec2 yx() const;\
	vec2 yy() const;\
	vec3 xxy() const;\
	vec3 xyx() const;\
	vec3 xyy() const;\
	vec3 yxx() const;\
	vec3 yxy() const;\
	vec3 yyx() const;\
	vec3 yyy() const;\
	vec4 xxxy() const;\
	vec4 xxyx() const;\
	vec4 xxyy() const;\
	vec4 xyxx() const;\
	vec4 xyxy() const;\
	vec4 xyyx() const;\
	vec4 xyyy() const;\
	vec4 yxxx() const;\
	vec4 yxxy() const;\
	vec4 yxyx() const;\
	vec4 yxyy() const;\
	vec4 yyxx() const;\
	vec4 yyxy() const;\
	vec4 yyyx() const;\
	vec4 yyyy() const;\
	vec2 xz() const;\
	vec2 yz() const;\
	vec2 zx() const;\
	vec2 zy() const;\
	vec2 zz() const;\
	vec3 xxz() const;\
	vec3 xyz() const;\
	vec3 xzx() const;\
	vec3 xzy() const;\
	vec3 xzz() const;\
	vec3 yxz() const;\
	vec3 yyz() const;\
	vec3 yzx() const;\
	vec3 yzy() const;\
	vec3 yzz() const;\
	vec3 zxx() const;\
	vec3 zxy() const;\
	vec3 zxz() const;\
	vec3 zyx() const;\
	vec3 zyy() const;\
	vec3 zyz() const;\
	vec3 zzx() const;\
	vec3 zzy() const;\
	vec3 zzz() const;\
	vec4 xxxz() const;\
	vec4 xxyz() const;\
	vec4 xxzx() const;\
	vec4 xxzy() const;\
	vec4 xxzz() const;\
	vec4 xyxz() const;\
	vec4 xyyz() const;\
	vec4 xyzx() const;\
	vec4 xyzy() const;\
	vec4 xyzz() const;\
	vec4 xzxx() const;\
	vec4 xzxy() const;\
	vec4 xzxz() const;\
	vec4 xzyx() const;\
	vec4 xzyy() const;\
	vec4 xzyz() const;\
	vec4 xzzx() const;\
	vec4 xzzy() const;\
	vec4 xzzz() const;\
	vec4 yxxz() const;\
	vec4 yxyz() const;\
	vec4 yxzx() const;\
	vec4 yxzy() const;\
	vec4 yxzz() const;\
	vec4 yyxz() const;\
	vec4 yyyz() const;\
	vec4 yyzx() const;\
	vec4 yyzy() const;\
	vec4 yyzz() const;\
	vec4 yzxx() const;\
	vec4 yzxy() const;\
	vec4 yzxz() const;\
	vec4 yzyx() const;\
	vec4 yzyy() const;\
	vec4 yzyz() const;\
	vec4 yzzx() const;\
	vec4 yzzy() const;\
	vec4 yzzz() const;\
	vec4 zxxx() const;\
	vec4 zxxy() const;\
	vec4 zxxz() const;\
	vec4 zxyx() const;\
	vec4 zxyy() const;\
	vec4 zxyz() const;\
	vec4 zxzx() const;\
	vec4 zxzy() const;\
	vec4 zxzz() const;\
	vec4 zyxx() const;\
	vec4 zyxy() const;\
	vec4 zyxz() const;\
	vec4 zyyx() const;\
	vec4 zyyy() const;\
	vec4 zyyz() const;\
	vec4 zyzx() const;\
	vec4 zyzy() const;\
	vec4 zyzz() const;\
	vec4 zzxx() const;\
	vec4 zzxy() const;\
	vec4 zzxz() const;\
	vec4 zzyx() const;\
	vec4 zzyy() const;\
	vec4 zzyz() const;\
	vec4 zzzx() const;\
	vec4 zzzy() const;\
	vec4 zzzz() const;\
	vec2 xw() const;\
	vec2 yw() const;\
	vec2 zw() const;\
	vec2 wx() const;\
	vec2 wy() const;\
	vec2 wz() const;\
	vec2 ww() const;\
	vec3 xxw() const;\
	vec3 xyw() const;\
	vec3 xzw() const;\
	vec3 xwx() const;\
	vec3 xwy() const;\
	vec3 xwz() const;\
	vec3 xww() const;\
	vec3 yxw() const;\
	vec3 yyw() const;\
	vec3 yzw() const;\
	vec3 ywx() const;\
	vec3 ywy() const;\
	vec3 ywz() const;\
	vec3 yww() const;\
	vec3 zxw() const;\
	vec3 zyw() const;\
	vec3 zzw() const;\
	vec3 zwx() const;\
	vec3 zwy() const;\
	vec3 zwz() const;\
	vec3 zww() const;\
	vec3 wxx() const;\
	vec3 wxy() const;\
	vec3 wxz() const;\
	vec3 wxw() const;\
	vec3 wyx() const;\
	vec3 wyy() const;\
	vec3 wyz() const;\
	vec3 wyw() const;\
	vec3 wzx() const;\
	vec3 wzy() const;\
	vec3 wzz() const;\
	vec3 wzw() const;\
	vec3 wwx() const;\
	vec3 wwy() const;\
	vec3 wwz() const;\
	vec3 www() const;\
	vec4 xxxw() const;\
	vec4 xxyw() const;\
	vec4 xxzw() const;\
	vec4 xxwx() const;\
	vec4 xxwy() const;\
	vec4 xxwz() const;\
	vec4 xxww() const;\
	vec4 xyxw() const;\
	vec4 xyyw() const;\
	vec4 xyzw() const;\
	vec4 xywx() const;\
	vec4 xywy() const;\
	vec4 xywz() const;\
	vec4 xyww() const;\
	vec4 xzxw() const;\
	vec4 xzyw() const;\
	vec4 xzzw() const;\
	vec4 xzwx() const;\
	vec4 xzwy() const;\
	vec4 xzwz() const;\
	vec4 xzww() const;\
	vec4 xwxx() const;\
	vec4 xwxy() const;\
	vec4 xwxz() const;\
	vec4 xwxw() const;\
	vec4 xwyx() const;\
	vec4 xwyy() const;\
	vec4 xwyz() const;\
	vec4 xwyw() const;\
	vec4 xwzx() const;\
	vec4 xwzy() const;\
	vec4 xwzz() const;\
	vec4 xwzw() const;\
	vec4 xwwx() const;\
	vec4 xwwy() const;\
	vec4 xwwz() const;\
	vec4 xwww() const;\
	vec4 yxxw() const;\
	vec4 yxyw() const;\
	vec4 yxzw() const;\
	vec4 yxwx() const;\
	vec4 yxwy() const;\
	vec4 yxwz() const;\
	vec4 yxww() const;\
	vec4 yyxw() const;\
	vec4 yyyw() const;\
	vec4 yyzw() const;\
	vec4 yywx() const;\
	vec4 yywy() const;\
	vec4 yywz() const;\
	vec4 yyww() const;\
	vec4 yzxw() const;\
	vec4 yzyw() const;\
	vec4 yzzw() const;\
	vec4 yzwx() const;\
	vec4 yzwy() const;\
	vec4 yzwz() const;\
	vec4 yzww() const;\
	vec4 ywxx() const;\
	vec4 ywxy() const;\
	vec4 ywxz() const;\
	vec4 ywxw() const;\
	vec4 ywyx() const;\
	vec4 ywyy() const;\
	vec4 ywyz() const;\
	vec4 ywyw() const;\
	vec4 ywzx() const;\
	vec4 ywzy() const;\
	vec4 ywzz() const;\
	vec4 ywzw() const;\
	vec4 ywwx() const;\
	vec4 ywwy() const;\
	vec4 ywwz() const;\
	vec4 ywww() const;\
	vec4 zxxw() const;\
	vec4 zxyw() const;\
	vec4 zxzw() const;\
	vec4 zxwx() const;\
	vec4 zxwy() const;\
	vec4 zxwz() const;\
	vec4 zxww() const;\
	vec4 zyxw() const;\
	vec4 zyyw() const;\
	vec4 zyzw() const;\
	vec4 zywx() const;\
	vec4 zywy() const;\
	vec4 zywz() const;\
	vec4 zyww() const;\
	vec4 zzxw() const;\
	vec4 zzyw() const;\
	vec4 zzzw() const;\
	vec4 zzwx() const;\
	vec4 zzwy() const;\
	vec4 zzwz() const;\
	vec4 zzww() const;\
	vec4 zwxx() const;\
	vec4 zwxy() const;\
	vec4 zwxz() const;\
	vec4 zwxw() const;\
	vec4 zwyx() const;\
	vec4 zwyy() const;\
	vec4 zwyz() const;\
	vec4 zwyw() const;\
	vec4 zwzx() const;\
	vec4 zwzy() const;\
	vec4 zwzz() const;\
	vec4 zwzw() const;\
	vec4 zwwx() const;\
	vec4 zwwy() const;\
	vec4 zwwz() const;\
	vec4 zwww() const;\
	vec4 wxxx() const;\
	vec4 wxxy() const;\
	vec4 wxxz() const;\
	vec4 wxxw() const;\
	vec4 wxyx() const;\
	vec4 wxyy() const;\
	vec4 wxyz() const;\
	vec4 wxyw() const;\
	vec4 wxzx() const;\
	vec4 wxzy() const;\
	vec4 wxzz() const;\
	vec4 wxzw() const;\
	vec4 wxwx() const;\
	vec4 wxwy() const;\
	vec4 wxwz() const;\
	vec4 wxww() const;\
	vec4 wyxx() const;\
	vec4 wyxy() const;\
	vec4 wyxz() const;\
	vec4 wyxw() const;\
	vec4 wyyx() const;\
	vec4 wyyy() const;\
	vec4 wyyz() const;\
	vec4 wyyw() const;\
	vec4 wyzx() const;\
	vec4 wyzy() const;\
	vec4 wyzz() const;\
	vec4 wyzw() const;\
	vec4 wywx() const;\
	vec4 wywy() const;\
	vec4 wywz() const;\
	vec4 wyww() const;\
	vec4 wzxx() const;\
	vec4 wzxy() const;\
	vec4 wzxz() const;\
	vec4 wzxw() const;\
	vec4 wzyx() const;\
	vec4 wzyy() const;\
	vec4 wzyz() const;\
	vec4 wzyw() const;\
	vec4 wzzx() const;\
	vec4 wzzy() const;\
	vec4 wzzz() const;\
	vec4 wzzw() const;\
	vec4 wzwx() const;\
	vec4 wzwy() const;\
	vec4 wzwz() const;\
	vec4 wzww() const;\
	vec4 wwxx() const;\
	vec4 wwxy() const;\
	vec4 wwxz() const;\
	vec4 wwxw() const;\
	vec4 wwyx() const;\
	vec4 wwyy() const;\
	vec4 wwyz() const;\
	vec4 wwyw() const;\
	vec4 wwzx() const;\
	vec4 wwzy() const;\
	vec4 wwzz() const;\
	vec4 wwzw() const;\
	vec4 wwwx() const;\
	vec4 wwwy() const;\
	vec4 wwwz() const;\
	vec4 wwww() const;\
	;

#define SWIZZLE_IMPL_FOR_VEC2() \
	inline vec2 vec2::xx() const{\
		return vec2(x, x);\
	}\
	inline vec3 vec2::xxx() const{\
		return vec3(x, x, x);\
	}\
	inline vec4 vec2::xxxx() const{\
		return vec4(x, x, x, x);\
	}\
	inline vec2 vec2::xy() const{\
		return vec2(x, y);\
	}\
	inline vec2 vec2::yx() const{\
		return vec2(y, x);\
	}\
	inline vec2 vec2::yy() const{\
		return vec2(y, y);\
	}\
	inline vec3 vec2::xxy() const{\
		return vec3(x, x, y);\
	}\
	inline vec3 vec2::xyx() const{\
		return vec3(x, y, x);\
	}\
	inline vec3 vec2::xyy() const{\
		return vec3(x, y, y);\
	}\
	inline vec3 vec2::yxx() const{\
		return vec3(y, x, x);\
	}\
	inline vec3 vec2::yxy() const{\
		return vec3(y, x, y);\
	}\
	inline vec3 vec2::yyx() const{\
		return vec3(y, y, x);\
	}\
	inline vec3 vec2::yyy() const{\
		return vec3(y, y, y);\
	}\
	inline vec4 vec2::xxxy() const{\
		return vec4(x, x, x, y);\
	}\
	inline vec4 vec2::xxyx() const{\
		return vec4(x, x, y, x);\
	}\
	inline vec4 vec2::xxyy() const{\
		return vec4(x, x, y, y);\
	}\
	inline vec4 vec2::xyxx() const{\
		return vec4(x, y, x, x);\
	}\
	inline vec4 vec2::xyxy() const{\
		return vec4(x, y, x, y);\
	}\
	inline vec4 vec2::xyyx() const{\
		return vec4(x, y, y, x);\
	}\
	inline vec4 vec2::xyyy() const{\
		return vec4(x, y, y, y);\
	}\
	inline vec4 vec2::yxxx() const{\
		return vec4(y, x, x, x);\
	}\
	inline vec4 vec2::yxxy() const{\
		return vec4(y, x, x, y);\
	}\
	inline vec4 vec2::yxyx() const{\
		return vec4(y, x, y, x);\
	}\
	inline vec4 vec2::yxyy() const{\
		return vec4(y, x, y, y);\
	}\
	inline vec4 vec2::yyxx() const{\
		return vec4(y, y, x, x);\
	}\
	inline vec4 vec2::yyxy() const{\
		return vec4(y, y, x, y);\
	}\
	inline vec4 vec2::yyyx() const{\
		return vec4(y, y, y, x);\
	}\
	inline vec4 vec2::yyyy() const{\
		return vec4(y, y, y, y);\
	}\
	;

#define SWIZZLE_IMPL_FOR_VEC3() \
	inline vec2 vec3::xx() const{\
		return vec2(x, x);\
	}\
	inline vec3 vec3::xxx() const{\
		return vec3(x, x, x);\
	}\
	inline vec4 vec3::xxxx() const{\
		return vec4(x, x, x, x);\
	}\
	inline vec2 vec3::xy() const{\
		return vec2(x, y);\
	}\
	inline vec2 vec3::yx() const{\
		return vec2(y, x);\
	}\
	inline vec2 vec3::yy() const{\
		return vec2(y, y);\
	}\
	inline vec3 vec3::xxy() const{\
		return vec3(x, x, y);\
	}\
	inline vec3 vec3::xyx() const{\
		return vec3(x, y, x);\
	}\
	inline vec3 vec3::xyy() const{\
		return vec3(x, y, y);\
	}\
	inline vec3 vec3::yxx() const{\
		return vec3(y, x, x);\
	}\
	inline vec3 vec3::yxy() const{\
		return vec3(y, x, y);\
	}\
	inline vec3 vec3::yyx() const{\
		return vec3(y, y, x);\
	}\
	inline vec3 vec3::yyy() const{\
		return vec3(y, y, y);\
	}\
	inline vec4 vec3::xxxy() const{\
		return vec4(x, x, x, y);\
	}\
	inline vec4 vec3::xxyx() const{\
		return vec4(x, x, y, x);\
	}\
	inline vec4 vec3::xxyy() const{\
		return vec4(x, x, y, y);\
	}\
	inline vec4 vec3::xyxx() const{\
		return vec4(x, y, x, x);\
	}\
	inline vec4 vec3::xyxy() const{\
		return vec4(x, y, x, y);\
	}\
	inline vec4 vec3::xyyx() const{\
		return vec4(x, y, y, x);\
	}\
	inline vec4 vec3::xyyy() const{\
		return vec4(x, y, y, y);\
	}\
	inline vec4 vec3::yxxx() const{\
		return vec4(y, x, x, x);\
	}\
	inline vec4 vec3::yxxy() const{\
		return vec4(y, x, x, y);\
	}\
	inline vec4 vec3::yxyx() const{\
		return vec4(y, x, y, x);\
	}\
	inline vec4 vec3::yxyy() const{\
		return vec4(y, x, y, y);\
	}\
	inline vec4 vec3::yyxx() const{\
		return vec4(y, y, x, x);\
	}\
	inline vec4 vec3::yyxy() const{\
		return vec4(y, y, x, y);\
	}\
	inline vec4 vec3::yyyx() const{\
		return vec4(y, y, y, x);\
	}\
	inline vec4 vec3::yyyy() const{\
		return vec4(y, y, y, y);\
	}\
	inline vec2 vec3::xz() const{\
		return vec2(x, z);\
	}\
	inline vec2 vec3::yz() const{\
		return vec2(y, z);\
	}\
	inline vec2 vec3::zx() const{\
		return vec2(z, x);\
	}\
	inline vec2 vec3::zy() const{\
		return vec2(z, y);\
	}\
	inline vec2 vec3::zz() const{\
		return vec2(z, z);\
	}\
	inline vec3 vec3::xxz() const{\
		return vec3(x, x, z);\
	}\
	inline vec3 vec3::xyz() const{\
		return vec3(x, y, z);\
	}\
	inline vec3 vec3::xzx() const{\
		return vec3(x, z, x);\
	}\
	inline vec3 vec3::xzy() const{\
		return vec3(x, z, y);\
	}\
	inline vec3 vec3::xzz() const{\
		return vec3(x, z, z);\
	}\
	inline vec3 vec3::yxz() const{\
		return vec3(y, x, z);\
	}\
	inline vec3 vec3::yyz() const{\
		return vec3(y, y, z);\
	}\
	inline vec3 vec3::yzx() const{\
		return vec3(y, z, x);\
	}\
	inline vec3 vec3::yzy() const{\
		return vec3(y, z, y);\
	}\
	inline vec3 vec3::yzz() const{\
		return vec3(y, z, z);\
	}\
	inline vec3 vec3::zxx() const{\
		return vec3(z, x, x);\
	}\
	inline vec3 vec3::zxy() const{\
		return vec3(z, x, y);\
	}\
	inline vec3 vec3::zxz() const{\
		return vec3(z, x, z);\
	}\
	inline vec3 vec3::zyx() const{\
		return vec3(z, y, x);\
	}\
	inline vec3 vec3::zyy() const{\
		return vec3(z, y, y);\
	}\
	inline vec3 vec3::zyz() const{\
		return vec3(z, y, z);\
	}\
	inline vec3 vec3::zzx() const{\
		return vec3(z, z, x);\
	}\
	inline vec3 vec3::zzy() const{\
		return vec3(z, z, y);\
	}\
	inline vec3 vec3::zzz() const{\
		return vec3(z, z, z);\
	}\
	inline vec4 vec3::xxxz() const{\
		return vec4(x, x, x, z);\
	}\
	inline vec4 vec3::xxyz() const{\
		return vec4(x, x, y, z);\
	}\
	inline vec4 vec3::xxzx() const{\
		return vec4(x, x, z, x);\
	}\
	inline vec4 vec3::xxzy() const{\
		return vec4(x, x, z, y);\
	}\
	inline vec4 vec3::xxzz() const{\
		return vec4(x, x, z, z);\
	}\
	inline vec4 vec3::xyxz() const{\
		return vec4(x, y, x, z);\
	}\
	inline vec4 vec3::xyyz() const{\
		return vec4(x, y, y, z);\
	}\
	inline vec4 vec3::xyzx() const{\
		return vec4(x, y, z, x);\
	}\
	inline vec4 vec3::xyzy() const{\
		return vec4(x, y, z, y);\
	}\
	inline vec4 vec3::xyzz() const{\
		return vec4(x, y, z, z);\
	}\
	inline vec4 vec3::xzxx() const{\
		return vec4(x, z, x, x);\
	}\
	inline vec4 vec3::xzxy() const{\
		return vec4(x, z, x, y);\
	}\
	inline vec4 vec3::xzxz() const{\
		return vec4(x, z, x, z);\
	}\
	inline vec4 vec3::xzyx() const{\
		return vec4(x, z, y, x);\
	}\
	inline vec4 vec3::xzyy() const{\
		return vec4(x, z, y, y);\
	}\
	inline vec4 vec3::xzyz() const{\
		return vec4(x, z, y, z);\
	}\
	inline vec4 vec3::xzzx() const{\
		return vec4(x, z, z, x);\
	}\
	inline vec4 vec3::xzzy() const{\
		return vec4(x, z, z, y);\
	}\
	inline vec4 vec3::xzzz() const{\
		return vec4(x, z, z, z);\
	}\
	inline vec4 vec3::yxxz() const{\
		return vec4(y, x, x, z);\
	}\
	inline vec4 vec3::yxyz() const{\
		return vec4(y, x, y, z);\
	}\
	inline vec4 vec3::yxzx() const{\
		return vec4(y, x, z, x);\
	}\
	inline vec4 vec3::yxzy() const{\
		return vec4(y, x, z, y);\
	}\
	inline vec4 vec3::yxzz() const{\
		return vec4(y, x, z, z);\
	}\
	inline vec4 vec3::yyxz() const{\
		return vec4(y, y, x, z);\
	}\
	inline vec4 vec3::yyyz() const{\
		return vec4(y, y, y, z);\
	}\
	inline vec4 vec3::yyzx() const{\
		return vec4(y, y, z, x);\
	}\
	inline vec4 vec3::yyzy() const{\
		return vec4(y, y, z, y);\
	}\
	inline vec4 vec3::yyzz() const{\
		return vec4(y, y, z, z);\
	}\
	inline vec4 vec3::yzxx() const{\
		return vec4(y, z, x, x);\
	}\
	inline vec4 vec3::yzxy() const{\
		return vec4(y, z, x, y);\
	}\
	inline vec4 vec3::yzxz() const{\
		return vec4(y, z, x, z);\
	}\
	inline vec4 vec3::yzyx() const{\
		return vec4(y, z, y, x);\
	}\
	inline vec4 vec3::yzyy() const{\
		return vec4(y, z, y, y);\
	}\
	inline vec4 vec3::yzyz() const{\
		return vec4(y, z, y, z);\
	}\
	inline vec4 vec3::yzzx() const{\
		return vec4(y, z, z, x);\
	}\
	inline vec4 vec3::yzzy() const{\
		return vec4(y, z, z, y);\
	}\
	inline vec4 vec3::yzzz() const{\
		return vec4(y, z, z, z);\
	}\
	inline vec4 vec3::zxxx() const{\
		return vec4(z, x, x, x);\
	}\
	inline vec4 vec3::zxxy() const{\
		return vec4(z, x, x, y);\
	}\
	inline vec4 vec3::zxxz() const{\
		return vec4(z, x, x, z);\
	}\
	inline vec4 vec3::zxyx() const{\
		return vec4(z, x, y, x);\
	}\
	inline vec4 vec3::zxyy() const{\
		return vec4(z, x, y, y);\
	}\
	inline vec4 vec3::zxyz() const{\
		return vec4(z, x, y, z);\
	}\
	inline vec4 vec3::zxzx() const{\
		return vec4(z, x, z, x);\
	}\
	inline vec4 vec3::zxzy() const{\
		return vec4(z, x, z, y);\
	}\
	inline vec4 vec3::zxzz() const{\
		return vec4(z, x, z, z);\
	}\
	inline vec4 vec3::zyxx() const{\
		return vec4(z, y, x, x);\
	}\
	inline vec4 vec3::zyxy() const{\
		return vec4(z, y, x, y);\
	}\
	inline vec4 vec3::zyxz() const{\
		return vec4(z, y, x, z);\
	}\
	inline vec4 vec3::zyyx() const{\
		return vec4(z, y, y, x);\
	}\
	inline vec4 vec3::zyyy() const{\
		return vec4(z, y, y, y);\
	}\
	inline vec4 vec3::zyyz() const{\
		return vec4(z, y, y, z);\
	}\
	inline vec4 vec3::zyzx() const{\
		return vec4(z, y, z, x);\
	}\
	inline vec4 vec3::zyzy() const{\
		return vec4(z, y, z, y);\
	}\
	inline vec4 vec3::zyzz() const{\
		return vec4(z, y, z, z);\
	}\
	inline vec4 vec3::zzxx() const{\
		return vec4(z, z, x, x);\
	}\
	inline vec4 vec3::zzxy() const{\
		return vec4(z, z, x, y);\
	}\
	inline vec4 vec3::zzxz() const{\
		return vec4(z, z, x, z);\
	}\
	inline vec4 vec3::zzyx() const{\
		return vec4(z, z, y, x);\
	}\
	inline vec4 vec3::zzyy() const{\
		return vec4(z, z, y, y);\
	}\
	inline vec4 vec3::zzyz() const{\
		return vec4(z, z, y, z);\
	}\
	inline vec4 vec3::zzzx() const{\
		return vec4(z, z, z, x);\
	}\
	inline vec4 vec3::zzzy() const{\
		return vec4(z, z, z, y);\
	}\
	inline vec4 vec3::zzzz() const{\
		return vec4(z, z, z, z);\
	}\
	;

#define SWIZZLE_IMPL_FOR_VEC4() \
	inline vec2 vec4::xx() const{\
		return vec2(x, x);\
	}\
	inline vec3 vec4::xxx() const{\
		return vec3(x, x, x);\
	}\
	inline vec4 vec4::xxxx() const{\
		return vec4(x, x, x, x);\
	}\
	inline vec2 vec4::xy() const{\
		return vec2(x, y);\
	}\
	inline vec2 vec4::yx() const{\
		return vec2(y, x);\
	}\
	inline vec2 vec4::yy() const{\
		return vec2(y, y);\
	}\
	inline vec3 vec4::xxy() const{\
		return vec3(x, x, y);\
	}\
	inline vec3 vec4::xyx() const{\
		return vec3(x, y, x);\
	}\
	inline vec3 vec4::xyy() const{\
		return vec3(x, y, y);\
	}\
	inline vec3 vec4::yxx() const{\
		return vec3(y, x, x);\
	}\
	inline vec3 vec4::yxy() const{\
		return vec3(y, x, y);\
	}\
	inline vec3 vec4::yyx() const{\
		return vec3(y, y, x);\
	}\
	inline vec3 vec4::yyy() const{\
		return vec3(y, y, y);\
	}\
	inline vec4 vec4::xxxy() const{\
		return vec4(x, x, x, y);\
	}\
	inline vec4 vec4::xxyx() const{\
		return vec4(x, x, y, x);\
	}\
	inline vec4 vec4::xxyy() const{\
		return vec4(x, x, y, y);\
	}\
	inline vec4 vec4::xyxx() const{\
		return vec4(x, y, x, x);\
	}\
	inline vec4 vec4::xyxy() const{\
		return vec4(x, y, x, y);\
	}\
	inline vec4 vec4::xyyx() const{\
		return vec4(x, y, y, x);\
	}\
	inline vec4 vec4::xyyy() const{\
		return vec4(x, y, y, y);\
	}\
	inline vec4 vec4::yxxx() const{\
		return vec4(y, x, x, x);\
	}\
	inline vec4 vec4::yxxy() const{\
		return vec4(y, x, x, y);\
	}\
	inline vec4 vec4::yxyx() const{\
		return vec4(y, x, y, x);\
	}\
	inline vec4 vec4::yxyy() const{\
		return vec4(y, x, y, y);\
	}\
	inline vec4 vec4::yyxx() const{\
		return vec4(y, y, x, x);\
	}\
	inline vec4 vec4::yyxy() const{\
		return vec4(y, y, x, y);\
	}\
	inline vec4 vec4::yyyx() const{\
		return vec4(y, y, y, x);\
	}\
	inline vec4 vec4::yyyy() const{\
		return vec4(y, y, y, y);\
	}\
	inline vec2 vec4::xz() const{\
		return vec2(x, z);\
	}\
	inline vec2 vec4::yz() const{\
		return vec2(y, z);\
	}\
	inline vec2 vec4::zx() const{\
		return vec2(z, x);\
	}\
	inline vec2 vec4::zy() const{\
		return vec2(z, y);\
	}\
	inline vec2 vec4::zz() const{\
		return vec2(z, z);\
	}\
	inline vec3 vec4::xxz() const{\
		return vec3(x, x, z);\
	}\
	inline vec3 vec4::xyz() const{\
		return vec3(x, y, z);\
	}\
	inline vec3 vec4::xzx() const{\
		return vec3(x, z, x);\
	}\
	inline vec3 vec4::xzy() const{\
		return vec3(x, z, y);\
	}\
	inline vec3 vec4::xzz() const{\
		return vec3(x, z, z);\
	}\
	inline vec3 vec4::yxz() const{\
		return vec3(y, x, z);\
	}\
	inline vec3 vec4::yyz() const{\
		return vec3(y, y, z);\
	}\
	inline vec3 vec4::yzx() const{\
		return vec3(y, z, x);\
	}\
	inline vec3 vec4::yzy() const{\
		return vec3(y, z, y);\
	}\
	inline vec3 vec4::yzz() const{\
		return vec3(y, z, z);\
	}\
	inline vec3 vec4::zxx() const{\
		return vec3(z, x, x);\
	}\
	inline vec3 vec4::zxy() const{\
		return vec3(z, x, y);\
	}\
	inline vec3 vec4::zxz() const{\
		return vec3(z, x, z);\
	}\
	inline vec3 vec4::zyx() const{\
		return vec3(z, y, x);\
	}\
	inline vec3 vec4::zyy() const{\
		return vec3(z, y, y);\
	}\
	inline vec3 vec4::zyz() const{\
		return vec3(z, y, z);\
	}\
	inline vec3 vec4::zzx() const{\
		return vec3(z, z, x);\
	}\
	inline vec3 vec4::zzy() const{\
		return vec3(z, z, y);\
	}\
	inline vec3 vec4::zzz() const{\
		return vec3(z, z, z);\
	}\
	inline vec4 vec4::xxxz() const{\
		return vec4(x, x, x, z);\
	}\
	inline vec4 vec4::xxyz() const{\
		return vec4(x, x, y, z);\
	}\
	inline vec4 vec4::xxzx() const{\
		return vec4(x, x, z, x);\
	}\
	inline vec4 vec4::xxzy() const{\
		return vec4(x, x, z, y);\
	}\
	inline vec4 vec4::xxzz() const{\
		return vec4(x, x, z, z);\
	}\
	inline vec4 vec4::xyxz() const{\
		return vec4(x, y, x, z);\
	}\
	inline vec4 vec4::xyyz() const{\
		return vec4(x, y, y, z);\
	}\
	inline vec4 vec4::xyzx() const{\
		return vec4(x, y, z, x);\
	}\
	inline vec4 vec4::xyzy() const{\
		return vec4(x, y, z, y);\
	}\
	inline vec4 vec4::xyzz() const{\
		return vec4(x, y, z, z);\
	}\
	inline vec4 vec4::xzxx() const{\
		return vec4(x, z, x, x);\
	}\
	inline vec4 vec4::xzxy() const{\
		return vec4(x, z, x, y);\
	}\
	inline vec4 vec4::xzxz() const{\
		return vec4(x, z, x, z);\
	}\
	inline vec4 vec4::xzyx() const{\
		return vec4(x, z, y, x);\
	}\
	inline vec4 vec4::xzyy() const{\
		return vec4(x, z, y, y);\
	}\
	inline vec4 vec4::xzyz() const{\
		return vec4(x, z, y, z);\
	}\
	inline vec4 vec4::xzzx() const{\
		return vec4(x, z, z, x);\
	}\
	inline vec4 vec4::xzzy() const{\
		return vec4(x, z, z, y);\
	}\
	inline vec4 vec4::xzzz() const{\
		return vec4(x, z, z, z);\
	}\
	inline vec4 vec4::yxxz() const{\
		return vec4(y, x, x, z);\
	}\
	inline vec4 vec4::yxyz() const{\
		return vec4(y, x, y, z);\
	}\
	inline vec4 vec4::yxzx() const{\
		return vec4(y, x, z, x);\
	}\
	inline vec4 vec4::yxzy() const{\
		return vec4(y, x, z, y);\
	}\
	inline vec4 vec4::yxzz() const{\
		return vec4(y, x, z, z);\
	}\
	inline vec4 vec4::yyxz() const{\
		return vec4(y, y, x, z);\
	}\
	inline vec4 vec4::yyyz() const{\
		return vec4(y, y, y, z);\
	}\
	inline vec4 vec4::yyzx() const{\
		return vec4(y, y, z, x);\
	}\
	inline vec4 vec4::yyzy() const{\
		return vec4(y, y, z, y);\
	}\
	inline vec4 vec4::yyzz() const{\
		return vec4(y, y, z, z);\
	}\
	inline vec4 vec4::yzxx() const{\
		return vec4(y, z, x, x);\
	}\
	inline vec4 vec4::yzxy() const{\
		return vec4(y, z, x, y);\
	}\
	inline vec4 vec4::yzxz() const{\
		return vec4(y, z, x, z);\
	}\
	inline vec4 vec4::yzyx() const{\
		return vec4(y, z, y, x);\
	}\
	inline vec4 vec4::yzyy() const{\
		return vec4(y, z, y, y);\
	}\
	inline vec4 vec4::yzyz() const{\
		return vec4(y, z, y, z);\
	}\
	inline vec4 vec4::yzzx() const{\
		return vec4(y, z, z, x);\
	}\
	inline vec4 vec4::yzzy() const{\
		return vec4(y, z, z, y);\
	}\
	inline vec4 vec4::yzzz() const{\
		return vec4(y, z, z, z);\
	}\
	inline vec4 vec4::zxxx() const{\
		return vec4(z, x, x, x);\
	}\
	inline vec4 vec4::zxxy() const{\
		return vec4(z, x, x, y);\
	}\
	inline vec4 vec4::zxxz() const{\
		return vec4(z, x, x, z);\
	}\
	inline vec4 vec4::zxyx() const{\
		return vec4(z, x, y, x);\
	}\
	inline vec4 vec4::zxyy() const{\
		return vec4(z, x, y, y);\
	}\
	inline vec4 vec4::zxyz() const{\
		return vec4(z, x, y, z);\
	}\
	inline vec4 vec4::zxzx() const{\
		return vec4(z, x, z, x);\
	}\
	inline vec4 vec4::zxzy() const{\
		return vec4(z, x, z, y);\
	}\
	inline vec4 vec4::zxzz() const{\
		return vec4(z, x, z, z);\
	}\
	inline vec4 vec4::zyxx() const{\
		return vec4(z, y, x, x);\
	}\
	inline vec4 vec4::zyxy() const{\
		return vec4(z, y, x, y);\
	}\
	inline vec4 vec4::zyxz() const{\
		return vec4(z, y, x, z);\
	}\
	inline vec4 vec4::zyyx() const{\
		return vec4(z, y, y, x);\
	}\
	inline vec4 vec4::zyyy() const{\
		return vec4(z, y, y, y);\
	}\
	inline vec4 vec4::zyyz() const{\
		return vec4(z, y, y, z);\
	}\
	inline vec4 vec4::zyzx() const{\
		return vec4(z, y, z, x);\
	}\
	inline vec4 vec4::zyzy() const{\
		return vec4(z, y, z, y);\
	}\
	inline vec4 vec4::zyzz() const{\
		return vec4(z, y, z, z);\
	}\
	inline vec4 vec4::zzxx() const{\
		return vec4(z, z, x, x);\
	}\
	inline vec4 vec4::zzxy() const{\
		return vec4(z, z, x, y);\
	}\
	inline vec4 vec4::zzxz() const{\
		return vec4(z, z, x, z);\
	}\
	inline vec4 vec4::zzyx() const{\
		return vec4(z, z, y, x);\
	}\
	inline vec4 vec4::zzyy() const{\
		return vec4(z, z, y, y);\
	}\
	inline vec4 vec4::zzyz() const{\
		return vec4(z, z, y, z);\
	}\
	inline vec4 vec4::zzzx() const{\
		return vec4(z, z, z, x);\
	}\
	inline vec4 vec4::zzzy() const{\
		return vec4(z, z, z, y);\
	}\
	inline vec4 vec4::zzzz() const{\
		return vec4(z, z, z, z);\
	}\
	inline vec2 vec4::xw() const{\
		return vec2(x, w);\
	}\
	inline vec2 vec4::yw() const{\
		return vec2(y, w);\
	}\
	inline vec2 vec4::zw() const{\
		return vec2(z, w);\
	}\
	inline vec2 vec4::wx() const{\
		return vec2(w, x);\
	}\
	inline vec2 vec4::wy() const{\
		return vec2(w, y);\
	}\
	inline vec2 vec4::wz() const{\
		return vec2(w, z);\
	}\
	inline vec2 vec4::ww() const{\
		return vec2(w, w);\
	}\
	inline vec3 vec4::xxw() const{\
		return vec3(x, x, w);\
	}\
	inline vec3 vec4::xyw() const{\
		return vec3(x, y, w);\
	}\
	inline vec3 vec4::xzw() const{\
		return vec3(x, z, w);\
	}\
	inline vec3 vec4::xwx() const{\
		return vec3(x, w, x);\
	}\
	inline vec3 vec4::xwy() const{\
		return vec3(x, w, y);\
	}\
	inline vec3 vec4::xwz() const{\
		return vec3(x, w, z);\
	}\
	inline vec3 vec4::xww() const{\
		return vec3(x, w, w);\
	}\
	inline vec3 vec4::yxw() const{\
		return vec3(y, x, w);\
	}\
	inline vec3 vec4::yyw() const{\
		return vec3(y, y, w);\
	}\
	inline vec3 vec4::yzw() const{\
		return vec3(y, z, w);\
	}\
	inline vec3 vec4::ywx() const{\
		return vec3(y, w, x);\
	}\
	inline vec3 vec4::ywy() const{\
		return vec3(y, w, y);\
	}\
	inline vec3 vec4::ywz() const{\
		return vec3(y, w, z);\
	}\
	inline vec3 vec4::yww() const{\
		return vec3(y, w, w);\
	}\
	inline vec3 vec4::zxw() const{\
		return vec3(z, x, w);\
	}\
	inline vec3 vec4::zyw() const{\
		return vec3(z, y, w);\
	}\
	inline vec3 vec4::zzw() const{\
		return vec3(z, z, w);\
	}\
	inline vec3 vec4::zwx() const{\
		return vec3(z, w, x);\
	}\
	inline vec3 vec4::zwy() const{\
		return vec3(z, w, y);\
	}\
	inline vec3 vec4::zwz() const{\
		return vec3(z, w, z);\
	}\
	inline vec3 vec4::zww() const{\
		return vec3(z, w, w);\
	}\
	inline vec3 vec4::wxx() const{\
		return vec3(w, x, x);\
	}\
	inline vec3 vec4::wxy() const{\
		return vec3(w, x, y);\
	}\
	inline vec3 vec4::wxz() const{\
		return vec3(w, x, z);\
	}\
	inline vec3 vec4::wxw() const{\
		return vec3(w, x, w);\
	}\
	inline vec3 vec4::wyx() const{\
		return vec3(w, y, x);\
	}\
	inline vec3 vec4::wyy() const{\
		return vec3(w, y, y);\
	}\
	inline vec3 vec4::wyz() const{\
		return vec3(w, y, z);\
	}\
	inline vec3 vec4::wyw() const{\
		return vec3(w, y, w);\
	}\
	inline vec3 vec4::wzx() const{\
		return vec3(w, z, x);\
	}\
	inline vec3 vec4::wzy() const{\
		return vec3(w, z, y);\
	}\
	inline vec3 vec4::wzz() const{\
		return vec3(w, z, z);\
	}\
	inline vec3 vec4::wzw() const{\
		return vec3(w, z, w);\
	}\
	inline vec3 vec4::wwx() const{\
		return vec3(w, w, x);\
	}\
	inline vec3 vec4::wwy() const{\
		return vec3(w, w, y);\
	}\
	inline vec3 vec4::wwz() const{\
		return vec3(w, w, z);\
	}\
	inline vec3 vec4::www() const{\
		return vec3(w, w, w);\
	}\
	inline vec4 vec4::xxxw() const{\
		return vec4(x, x, x, w);\
	}\
	inline vec4 vec4::xxyw() const{\
		return vec4(x, x, y, w);\
	}\
	inline vec4 vec4::xxzw() const{\
		return vec4(x, x, z, w);\
	}\
	inline vec4 vec4::xxwx() const{\
		return vec4(x, x, w, x);\
	}\
	inline vec4 vec4::xxwy() const{\
		return vec4(x, x, w, y);\
	}\
	inline vec4 vec4::xxwz() const{\
		return vec4(x, x, w, z);\
	}\
	inline vec4 vec4::xxww() const{\
		return vec4(x, x, w, w);\
	}\
	inline vec4 vec4::xyxw() const{\
		return vec4(x, y, x, w);\
	}\
	inline vec4 vec4::xyyw() const{\
		return vec4(x, y, y, w);\
	}\
	inline vec4 vec4::xyzw() const{\
		return vec4(x, y, z, w);\
	}\
	inline vec4 vec4::xywx() const{\
		return vec4(x, y, w, x);\
	}\
	inline vec4 vec4::xywy() const{\
		return vec4(x, y, w, y);\
	}\
	inline vec4 vec4::xywz() const{\
		return vec4(x, y, w, z);\
	}\
	inline vec4 vec4::xyww() const{\
		return vec4(x, y, w, w);\
	}\
	inline vec4 vec4::xzxw() const{\
		return vec4(x, z, x, w);\
	}\
	inline vec4 vec4::xzyw() const{\
		return vec4(x, z, y, w);\
	}\
	inline vec4 vec4::xzzw() const{\
		return vec4(x, z, z, w);\
	}\
	inline vec4 vec4::xzwx() const{\
		return vec4(x, z, w, x);\
	}\
	inline vec4 vec4::xzwy() const{\
		return vec4(x, z, w, y);\
	}\
	inline vec4 vec4::xzwz() const{\
		return vec4(x, z, w, z);\
	}\
	inline vec4 vec4::xzww() const{\
		return vec4(x, z, w, w);\
	}\
	inline vec4 vec4::xwxx() const{\
		return vec4(x, w, x, x);\
	}\
	inline vec4 vec4::xwxy() const{\
		return vec4(x, w, x, y);\
	}\
	inline vec4 vec4::xwxz() const{\
		return vec4(x, w, x, z);\
	}\
	inline vec4 vec4::xwxw() const{\
		return vec4(x, w, x, w);\
	}\
	inline vec4 vec4::xwyx() const{\
		return vec4(x, w, y, x);\
	}\
	inline vec4 vec4::xwyy() const{\
		return vec4(x, w, y, y);\
	}\
	inline vec4 vec4::xwyz() const{\
		return vec4(x, w, y, z);\
	}\
	inline vec4 vec4::xwyw() const{\
		return vec4(x, w, y, w);\
	}\
	inline vec4 vec4::xwzx() const{\
		return vec4(x, w, z, x);\
	}\
	inline vec4 vec4::xwzy() const{\
		return vec4(x, w, z, y);\
	}\
	inline vec4 vec4::xwzz() const{\
		return vec4(x, w, z, z);\
	}\
	inline vec4 vec4::xwzw() const{\
		return vec4(x, w, z, w);\
	}\
	inline vec4 vec4::xwwx() const{\
		return vec4(x, w, w, x);\
	}\
	inline vec4 vec4::xwwy() const{\
		return vec4(x, w, w, y);\
	}\
	inline vec4 vec4::xwwz() const{\
		return vec4(x, w, w, z);\
	}\
	inline vec4 vec4::xwww() const{\
		return vec4(x, w, w, w);\
	}\
	inline vec4 vec4::yxxw() const{\
		return vec4(y, x, x, w);\
	}\
	inline vec4 vec4::yxyw() const{\
		return vec4(y, x, y, w);\
	}\
	inline vec4 vec4::yxzw() const{\
		return vec4(y, x, z, w);\
	}\
	inline vec4 vec4::yxwx() const{\
		return vec4(y, x, w, x);\
	}\
	inline vec4 vec4::yxwy() const{\
		return vec4(y, x, w, y);\
	}\
	inline vec4 vec4::yxwz() const{\
		return vec4(y, x, w, z);\
	}\
	inline vec4 vec4::yxww() const{\
		return vec4(y, x, w, w);\
	}\
	inline vec4 vec4::yyxw() const{\
		return vec4(y, y, x, w);\
	}\
	inline vec4 vec4::yyyw() const{\
		return vec4(y, y, y, w);\
	}\
	inline vec4 vec4::yyzw() const{\
		return vec4(y, y, z, w);\
	}\
	inline vec4 vec4::yywx() const{\
		return vec4(y, y, w, x);\
	}\
	inline vec4 vec4::yywy() const{\
		return vec4(y, y, w, y);\
	}\
	inline vec4 vec4::yywz() const{\
		return vec4(y, y, w, z);\
	}\
	inline vec4 vec4::yyww() const{\
		return vec4(y, y, w, w);\
	}\
	inline vec4 vec4::yzxw() const{\
		return vec4(y, z, x, w);\
	}\
	inline vec4 vec4::yzyw() const{\
		return vec4(y, z, y, w);\
	}\
	inline vec4 vec4::yzzw() const{\
		return vec4(y, z, z, w);\
	}\
	inline vec4 vec4::yzwx() const{\
		return vec4(y, z, w, x);\
	}\
	inline vec4 vec4::yzwy() const{\
		return vec4(y, z, w, y);\
	}\
	inline vec4 vec4::yzwz() const{\
		return vec4(y, z, w, z);\
	}\
	inline vec4 vec4::yzww() const{\
		return vec4(y, z, w, w);\
	}\
	inline vec4 vec4::ywxx() const{\
		return vec4(y, w, x, x);\
	}\
	inline vec4 vec4::ywxy() const{\
		return vec4(y, w, x, y);\
	}\
	inline vec4 vec4::ywxz() const{\
		return vec4(y, w, x, z);\
	}\
	inline vec4 vec4::ywxw() const{\
		return vec4(y, w, x, w);\
	}\
	inline vec4 vec4::ywyx() const{\
		return vec4(y, w, y, x);\
	}\
	inline vec4 vec4::ywyy() const{\
		return vec4(y, w, y, y);\
	}\
	inline vec4 vec4::ywyz() const{\
		return vec4(y, w, y, z);\
	}\
	inline vec4 vec4::ywyw() const{\
		return vec4(y, w, y, w);\
	}\
	inline vec4 vec4::ywzx() const{\
		return vec4(y, w, z, x);\
	}\
	inline vec4 vec4::ywzy() const{\
		return vec4(y, w, z, y);\
	}\
	inline vec4 vec4::ywzz() const{\
		return vec4(y, w, z, z);\
	}\
	inline vec4 vec4::ywzw() const{\
		return vec4(y, w, z, w);\
	}\
	inline vec4 vec4::ywwx() const{\
		return vec4(y, w, w, x);\
	}\
	inline vec4 vec4::ywwy() const{\
		return vec4(y, w, w, y);\
	}\
	inline vec4 vec4::ywwz() const{\
		return vec4(y, w, w, z);\
	}\
	inline vec4 vec4::ywww() const{\
		return vec4(y, w, w, w);\
	}\
	inline vec4 vec4::zxxw() const{\
		return vec4(z, x, x, w);\
	}\
	inline vec4 vec4::zxyw() const{\
		return vec4(z, x, y, w);\
	}\
	inline vec4 vec4::zxzw() const{\
		return vec4(z, x, z, w);\
	}\
	inline vec4 vec4::zxwx() const{\
		return vec4(z, x, w, x);\
	}\
	inline vec4 vec4::zxwy() const{\
		return vec4(z, x, w, y);\
	}\
	inline vec4 vec4::zxwz() const{\
		return vec4(z, x, w, z);\
	}\
	inline vec4 vec4::zxww() const{\
		return vec4(z, x, w, w);\
	}\
	inline vec4 vec4::zyxw() const{\
		return vec4(z, y, x, w);\
	}\
	inline vec4 vec4::zyyw() const{\
		return vec4(z, y, y, w);\
	}\
	inline vec4 vec4::zyzw() const{\
		return vec4(z, y, z, w);\
	}\
	inline vec4 vec4::zywx() const{\
		return vec4(z, y, w, x);\
	}\
	inline vec4 vec4::zywy() const{\
		return vec4(z, y, w, y);\
	}\
	inline vec4 vec4::zywz() const{\
		return vec4(z, y, w, z);\
	}\
	inline vec4 vec4::zyww() const{\
		return vec4(z, y, w, w);\
	}\
	inline vec4 vec4::zzxw() const{\
		return vec4(z, z, x, w);\
	}\
	inline vec4 vec4::zzyw() const{\
		return vec4(z, z, y, w);\
	}\
	inline vec4 vec4::zzzw() const{\
		return vec4(z, z, z, w);\
	}\
	inline vec4 vec4::zzwx() const{\
		return vec4(z, z, w, x);\
	}\
	inline vec4 vec4::zzwy() const{\
		return vec4(z, z, w, y);\
	}\
	inline vec4 vec4::zzwz() const{\
		return vec4(z, z, w, z);\
	}\
	inline vec4 vec4::zzww() const{\
		return vec4(z, z, w, w);\
	}\
	inline vec4 vec4::zwxx() const{\
		return vec4(z, w, x, x);\
	}\
	inline vec4 vec4::zwxy() const{\
		return vec4(z, w, x, y);\
	}\
	inline vec4 vec4::zwxz() const{\
		return vec4(z, w, x, z);\
	}\
	inline vec4 vec4::zwxw() const{\
		return vec4(z, w, x, w);\
	}\
	inline vec4 vec4::zwyx() const{\
		return vec4(z, w, y, x);\
	}\
	inline vec4 vec4::zwyy() const{\
		return vec4(z, w, y, y);\
	}\
	inline vec4 vec4::zwyz() const{\
		return vec4(z, w, y, z);\
	}\
	inline vec4 vec4::zwyw() const{\
		return vec4(z, w, y, w);\
	}\
	inline vec4 vec4::zwzx() const{\
		return vec4(z, w, z, x);\
	}\
	inline vec4 vec4::zwzy() const{\
		return vec4(z, w, z, y);\
	}\
	inline vec4 vec4::zwzz() const{\
		return vec4(z, w, z, z);\
	}\
	inline vec4 vec4::zwzw() const{\
		return vec4(z, w, z, w);\
	}\
	inline vec4 vec4::zwwx() const{\
		return vec4(z, w, w, x);\
	}\
	inline vec4 vec4::zwwy() const{\
		return vec4(z, w, w, y);\
	}\
	inline vec4 vec4::zwwz() const{\
		return vec4(z, w, w, z);\
	}\
	inline vec4 vec4::zwww() const{\
		return vec4(z, w, w, w);\
	}\
	inline vec4 vec4::wxxx() const{\
		return vec4(w, x, x, x);\
	}\
	inline vec4 vec4::wxxy() const{\
		return vec4(w, x, x, y);\
	}\
	inline vec4 vec4::wxxz() const{\
		return vec4(w, x, x, z);\
	}\
	inline vec4 vec4::wxxw() const{\
		return vec4(w, x, x, w);\
	}\
	inline vec4 vec4::wxyx() const{\
		return vec4(w, x, y, x);\
	}\
	inline vec4 vec4::wxyy() const{\
		return vec4(w, x, y, y);\
	}\
	inline vec4 vec4::wxyz() const{\
		return vec4(w, x, y, z);\
	}\
	inline vec4 vec4::wxyw() const{\
		return vec4(w, x, y, w);\
	}\
	inline vec4 vec4::wxzx() const{\
		return vec4(w, x, z, x);\
	}\
	inline vec4 vec4::wxzy() const{\
		return vec4(w, x, z, y);\
	}\
	inline vec4 vec4::wxzz() const{\
		return vec4(w, x, z, z);\
	}\
	inline vec4 vec4::wxzw() const{\
		return vec4(w, x, z, w);\
	}\
	inline vec4 vec4::wxwx() const{\
		return vec4(w, x, w, x);\
	}\
	inline vec4 vec4::wxwy() const{\
		return vec4(w, x, w, y);\
	}\
	inline vec4 vec4::wxwz() const{\
		return vec4(w, x, w, z);\
	}\
	inline vec4 vec4::wxww() const{\
		return vec4(w, x, w, w);\
	}\
	inline vec4 vec4::wyxx() const{\
		return vec4(w, y, x, x);\
	}\
	inline vec4 vec4::wyxy() const{\
		return vec4(w, y, x, y);\
	}\
	inline vec4 vec4::wyxz() const{\
		return vec4(w, y, x, z);\
	}\
	inline vec4 vec4::wyxw() const{\
		return vec4(w, y, x, w);\
	}\
	inline vec4 vec4::wyyx() const{\
		return vec4(w, y, y, x);\
	}\
	inline vec4 vec4::wyyy() const{\
		return vec4(w, y, y, y);\
	}\
	inline vec4 vec4::wyyz() const{\
		return vec4(w, y, y, z);\
	}\
	inline vec4 vec4::wyyw() const{\
		return vec4(w, y, y, w);\
	}\
	inline vec4 vec4::wyzx() const{\
		return vec4(w, y, z, x);\
	}\
	inline vec4 vec4::wyzy() const{\
		return vec4(w, y, z, y);\
	}\
	inline vec4 vec4::wyzz() const{\
		return vec4(w, y, z, z);\
	}\
	inline vec4 vec4::wyzw() const{\
		return vec4(w, y, z, w);\
	}\
	inline vec4 vec4::wywx() const{\
		return vec4(w, y, w, x);\
	}\
	inline vec4 vec4::wywy() const{\
		return vec4(w, y, w, y);\
	}\
	inline vec4 vec4::wywz() const{\
		return vec4(w, y, w, z);\
	}\
	inline vec4 vec4::wyww() const{\
		return vec4(w, y, w, w);\
	}\
	inline vec4 vec4::wzxx() const{\
		return vec4(w, z, x, x);\
	}\
	inline vec4 vec4::wzxy() const{\
		return vec4(w, z, x, y);\
	}\
	inline vec4 vec4::wzxz() const{\
		return vec4(w, z, x, z);\
	}\
	inline vec4 vec4::wzxw() const{\
		return vec4(w, z, x, w);\
	}\
	inline vec4 vec4::wzyx() const{\
		return vec4(w, z, y, x);\
	}\
	inline vec4 vec4::wzyy() const{\
		return vec4(w, z, y, y);\
	}\
	inline vec4 vec4::wzyz() const{\
		return vec4(w, z, y, z);\
	}\
	inline vec4 vec4::wzyw() const{\
		return vec4(w, z, y, w);\
	}\
	inline vec4 vec4::wzzx() const{\
		return vec4(w, z, z, x);\
	}\
	inline vec4 vec4::wzzy() const{\
		return vec4(w, z, z, y);\
	}\
	inline vec4 vec4::wzzz() const{\
		return vec4(w, z, z, z);\
	}\
	inline vec4 vec4::wzzw() const{\
		return vec4(w, z, z, w);\
	}\
	inline vec4 vec4::wzwx() const{\
		return vec4(w, z, w, x);\
	}\
	inline vec4 vec4::wzwy() const{\
		return vec4(w, z, w, y);\
	}\
	inline vec4 vec4::wzwz() const{\
		return vec4(w, z, w, z);\
	}\
	inline vec4 vec4::wzww() const{\
		return vec4(w, z, w, w);\
	}\
	inline vec4 vec4::wwxx() const{\
		return vec4(w, w, x, x);\
	}\
	inline vec4 vec4::wwxy() const{\
		return vec4(w, w, x, y);\
	}\
	inline vec4 vec4::wwxz() const{\
		return vec4(w, w, x, z);\
	}\
	inline vec4 vec4::wwxw() const{\
		return vec4(w, w, x, w);\
	}\
	inline vec4 vec4::wwyx() const{\
		return vec4(w, w, y, x);\
	}\
	inline vec4 vec4::wwyy() const{\
		return vec4(w, w, y, y);\
	}\
	inline vec4 vec4::wwyz() const{\
		return vec4(w, w, y, z);\
	}\
	inline vec4 vec4::wwyw() const{\
		return vec4(w, w, y, w);\
	}\
	inline vec4 vec4::wwzx() const{\
		return vec4(w, w, z, x);\
	}\
	inline vec4 vec4::wwzy() const{\
		return vec4(w, w, z, y);\
	}\
	inline vec4 vec4::wwzz() const{\
		return vec4(w, w, z, z);\
	}\
	inline vec4 vec4::wwzw() const{\
		return vec4(w, w, z, w);\
	}\
	inline vec4 vec4::wwwx() const{\
		return vec4(w, w, w, x);\
	}\
	inline vec4 vec4::wwwy() const{\
		return vec4(w, w, w, y);\
	}\
	inline vec4 vec4::wwwz() const{\
		return vec4(w, w, w, z);\
	}\
	inline vec4 vec4::wwww() const{\
		return vec4(w, w, w, w);\
	}\
	;

#endif