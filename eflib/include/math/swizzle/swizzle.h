#ifndef EFLIB_SWIZZLE_H
#define EFLIB_SWIZZLE_H

#define SWIZZLE_DECL_FOR_VEC2() \
	ScalarT const& x() const;\
	ScalarT& x();\
	vector_<ScalarT,2> xx() const;\
	vector_<ScalarT,3> xxx() const;\
	vector_<ScalarT,4> xxxx() const;\
	ScalarT const& y() const;\
	ScalarT& y();\
	vector_<ScalarT,2> xy() const;\
	vector_<ScalarT,2> yx() const;\
	vector_<ScalarT,2> yy() const;\
	vector_<ScalarT,3> xxy() const;\
	vector_<ScalarT,3> xyx() const;\
	vector_<ScalarT,3> xyy() const;\
	vector_<ScalarT,3> yxx() const;\
	vector_<ScalarT,3> yxy() const;\
	vector_<ScalarT,3> yyx() const;\
	vector_<ScalarT,3> yyy() const;\
	vector_<ScalarT,4> xxxy() const;\
	vector_<ScalarT,4> xxyx() const;\
	vector_<ScalarT,4> xxyy() const;\
	vector_<ScalarT,4> xyxx() const;\
	vector_<ScalarT,4> xyxy() const;\
	vector_<ScalarT,4> xyyx() const;\
	vector_<ScalarT,4> xyyy() const;\
	vector_<ScalarT,4> yxxx() const;\
	vector_<ScalarT,4> yxxy() const;\
	vector_<ScalarT,4> yxyx() const;\
	vector_<ScalarT,4> yxyy() const;\
	vector_<ScalarT,4> yyxx() const;\
	vector_<ScalarT,4> yyxy() const;\
	vector_<ScalarT,4> yyyx() const;\
	vector_<ScalarT,4> yyyy() const;\
	;

#define SWIZZLE_DECL_FOR_VEC3() \
	ScalarT const& x() const;\
	ScalarT& x();\
	vector_<ScalarT,2> xx() const;\
	vector_<ScalarT,3> xxx() const;\
	vector_<ScalarT,4> xxxx() const;\
	ScalarT const& y() const;\
	ScalarT& y();\
	vector_<ScalarT,2> xy() const;\
	vector_<ScalarT,2> yx() const;\
	vector_<ScalarT,2> yy() const;\
	vector_<ScalarT,3> xxy() const;\
	vector_<ScalarT,3> xyx() const;\
	vector_<ScalarT,3> xyy() const;\
	vector_<ScalarT,3> yxx() const;\
	vector_<ScalarT,3> yxy() const;\
	vector_<ScalarT,3> yyx() const;\
	vector_<ScalarT,3> yyy() const;\
	vector_<ScalarT,4> xxxy() const;\
	vector_<ScalarT,4> xxyx() const;\
	vector_<ScalarT,4> xxyy() const;\
	vector_<ScalarT,4> xyxx() const;\
	vector_<ScalarT,4> xyxy() const;\
	vector_<ScalarT,4> xyyx() const;\
	vector_<ScalarT,4> xyyy() const;\
	vector_<ScalarT,4> yxxx() const;\
	vector_<ScalarT,4> yxxy() const;\
	vector_<ScalarT,4> yxyx() const;\
	vector_<ScalarT,4> yxyy() const;\
	vector_<ScalarT,4> yyxx() const;\
	vector_<ScalarT,4> yyxy() const;\
	vector_<ScalarT,4> yyyx() const;\
	vector_<ScalarT,4> yyyy() const;\
	ScalarT const& z() const;\
	ScalarT& z();\
	vector_<ScalarT,2> xz() const;\
	vector_<ScalarT,2> yz() const;\
	vector_<ScalarT,2> zx() const;\
	vector_<ScalarT,2> zy() const;\
	vector_<ScalarT,2> zz() const;\
	vector_<ScalarT,3> xxz() const;\
	vector_<ScalarT,3> xyz() const;\
	vector_<ScalarT,3> xzx() const;\
	vector_<ScalarT,3> xzy() const;\
	vector_<ScalarT,3> xzz() const;\
	vector_<ScalarT,3> yxz() const;\
	vector_<ScalarT,3> yyz() const;\
	vector_<ScalarT,3> yzx() const;\
	vector_<ScalarT,3> yzy() const;\
	vector_<ScalarT,3> yzz() const;\
	vector_<ScalarT,3> zxx() const;\
	vector_<ScalarT,3> zxy() const;\
	vector_<ScalarT,3> zxz() const;\
	vector_<ScalarT,3> zyx() const;\
	vector_<ScalarT,3> zyy() const;\
	vector_<ScalarT,3> zyz() const;\
	vector_<ScalarT,3> zzx() const;\
	vector_<ScalarT,3> zzy() const;\
	vector_<ScalarT,3> zzz() const;\
	vector_<ScalarT,4> xxxz() const;\
	vector_<ScalarT,4> xxyz() const;\
	vector_<ScalarT,4> xxzx() const;\
	vector_<ScalarT,4> xxzy() const;\
	vector_<ScalarT,4> xxzz() const;\
	vector_<ScalarT,4> xyxz() const;\
	vector_<ScalarT,4> xyyz() const;\
	vector_<ScalarT,4> xyzx() const;\
	vector_<ScalarT,4> xyzy() const;\
	vector_<ScalarT,4> xyzz() const;\
	vector_<ScalarT,4> xzxx() const;\
	vector_<ScalarT,4> xzxy() const;\
	vector_<ScalarT,4> xzxz() const;\
	vector_<ScalarT,4> xzyx() const;\
	vector_<ScalarT,4> xzyy() const;\
	vector_<ScalarT,4> xzyz() const;\
	vector_<ScalarT,4> xzzx() const;\
	vector_<ScalarT,4> xzzy() const;\
	vector_<ScalarT,4> xzzz() const;\
	vector_<ScalarT,4> yxxz() const;\
	vector_<ScalarT,4> yxyz() const;\
	vector_<ScalarT,4> yxzx() const;\
	vector_<ScalarT,4> yxzy() const;\
	vector_<ScalarT,4> yxzz() const;\
	vector_<ScalarT,4> yyxz() const;\
	vector_<ScalarT,4> yyyz() const;\
	vector_<ScalarT,4> yyzx() const;\
	vector_<ScalarT,4> yyzy() const;\
	vector_<ScalarT,4> yyzz() const;\
	vector_<ScalarT,4> yzxx() const;\
	vector_<ScalarT,4> yzxy() const;\
	vector_<ScalarT,4> yzxz() const;\
	vector_<ScalarT,4> yzyx() const;\
	vector_<ScalarT,4> yzyy() const;\
	vector_<ScalarT,4> yzyz() const;\
	vector_<ScalarT,4> yzzx() const;\
	vector_<ScalarT,4> yzzy() const;\
	vector_<ScalarT,4> yzzz() const;\
	vector_<ScalarT,4> zxxx() const;\
	vector_<ScalarT,4> zxxy() const;\
	vector_<ScalarT,4> zxxz() const;\
	vector_<ScalarT,4> zxyx() const;\
	vector_<ScalarT,4> zxyy() const;\
	vector_<ScalarT,4> zxyz() const;\
	vector_<ScalarT,4> zxzx() const;\
	vector_<ScalarT,4> zxzy() const;\
	vector_<ScalarT,4> zxzz() const;\
	vector_<ScalarT,4> zyxx() const;\
	vector_<ScalarT,4> zyxy() const;\
	vector_<ScalarT,4> zyxz() const;\
	vector_<ScalarT,4> zyyx() const;\
	vector_<ScalarT,4> zyyy() const;\
	vector_<ScalarT,4> zyyz() const;\
	vector_<ScalarT,4> zyzx() const;\
	vector_<ScalarT,4> zyzy() const;\
	vector_<ScalarT,4> zyzz() const;\
	vector_<ScalarT,4> zzxx() const;\
	vector_<ScalarT,4> zzxy() const;\
	vector_<ScalarT,4> zzxz() const;\
	vector_<ScalarT,4> zzyx() const;\
	vector_<ScalarT,4> zzyy() const;\
	vector_<ScalarT,4> zzyz() const;\
	vector_<ScalarT,4> zzzx() const;\
	vector_<ScalarT,4> zzzy() const;\
	vector_<ScalarT,4> zzzz() const;\
	;

#define SWIZZLE_DECL_FOR_VEC4() \
	ScalarT const& x() const;\
	ScalarT& x();\
	vector_<ScalarT,2> xx() const;\
	vector_<ScalarT,3> xxx() const;\
	vector_<ScalarT,4> xxxx() const;\
	ScalarT const& y() const;\
	ScalarT& y();\
	vector_<ScalarT,2> xy() const;\
	vector_<ScalarT,2> yx() const;\
	vector_<ScalarT,2> yy() const;\
	vector_<ScalarT,3> xxy() const;\
	vector_<ScalarT,3> xyx() const;\
	vector_<ScalarT,3> xyy() const;\
	vector_<ScalarT,3> yxx() const;\
	vector_<ScalarT,3> yxy() const;\
	vector_<ScalarT,3> yyx() const;\
	vector_<ScalarT,3> yyy() const;\
	vector_<ScalarT,4> xxxy() const;\
	vector_<ScalarT,4> xxyx() const;\
	vector_<ScalarT,4> xxyy() const;\
	vector_<ScalarT,4> xyxx() const;\
	vector_<ScalarT,4> xyxy() const;\
	vector_<ScalarT,4> xyyx() const;\
	vector_<ScalarT,4> xyyy() const;\
	vector_<ScalarT,4> yxxx() const;\
	vector_<ScalarT,4> yxxy() const;\
	vector_<ScalarT,4> yxyx() const;\
	vector_<ScalarT,4> yxyy() const;\
	vector_<ScalarT,4> yyxx() const;\
	vector_<ScalarT,4> yyxy() const;\
	vector_<ScalarT,4> yyyx() const;\
	vector_<ScalarT,4> yyyy() const;\
	ScalarT const& z() const;\
	ScalarT& z();\
	vector_<ScalarT,2> xz() const;\
	vector_<ScalarT,2> yz() const;\
	vector_<ScalarT,2> zx() const;\
	vector_<ScalarT,2> zy() const;\
	vector_<ScalarT,2> zz() const;\
	vector_<ScalarT,3> xxz() const;\
	vector_<ScalarT,3> xyz() const;\
	vector_<ScalarT,3> xzx() const;\
	vector_<ScalarT,3> xzy() const;\
	vector_<ScalarT,3> xzz() const;\
	vector_<ScalarT,3> yxz() const;\
	vector_<ScalarT,3> yyz() const;\
	vector_<ScalarT,3> yzx() const;\
	vector_<ScalarT,3> yzy() const;\
	vector_<ScalarT,3> yzz() const;\
	vector_<ScalarT,3> zxx() const;\
	vector_<ScalarT,3> zxy() const;\
	vector_<ScalarT,3> zxz() const;\
	vector_<ScalarT,3> zyx() const;\
	vector_<ScalarT,3> zyy() const;\
	vector_<ScalarT,3> zyz() const;\
	vector_<ScalarT,3> zzx() const;\
	vector_<ScalarT,3> zzy() const;\
	vector_<ScalarT,3> zzz() const;\
	vector_<ScalarT,4> xxxz() const;\
	vector_<ScalarT,4> xxyz() const;\
	vector_<ScalarT,4> xxzx() const;\
	vector_<ScalarT,4> xxzy() const;\
	vector_<ScalarT,4> xxzz() const;\
	vector_<ScalarT,4> xyxz() const;\
	vector_<ScalarT,4> xyyz() const;\
	vector_<ScalarT,4> xyzx() const;\
	vector_<ScalarT,4> xyzy() const;\
	vector_<ScalarT,4> xyzz() const;\
	vector_<ScalarT,4> xzxx() const;\
	vector_<ScalarT,4> xzxy() const;\
	vector_<ScalarT,4> xzxz() const;\
	vector_<ScalarT,4> xzyx() const;\
	vector_<ScalarT,4> xzyy() const;\
	vector_<ScalarT,4> xzyz() const;\
	vector_<ScalarT,4> xzzx() const;\
	vector_<ScalarT,4> xzzy() const;\
	vector_<ScalarT,4> xzzz() const;\
	vector_<ScalarT,4> yxxz() const;\
	vector_<ScalarT,4> yxyz() const;\
	vector_<ScalarT,4> yxzx() const;\
	vector_<ScalarT,4> yxzy() const;\
	vector_<ScalarT,4> yxzz() const;\
	vector_<ScalarT,4> yyxz() const;\
	vector_<ScalarT,4> yyyz() const;\
	vector_<ScalarT,4> yyzx() const;\
	vector_<ScalarT,4> yyzy() const;\
	vector_<ScalarT,4> yyzz() const;\
	vector_<ScalarT,4> yzxx() const;\
	vector_<ScalarT,4> yzxy() const;\
	vector_<ScalarT,4> yzxz() const;\
	vector_<ScalarT,4> yzyx() const;\
	vector_<ScalarT,4> yzyy() const;\
	vector_<ScalarT,4> yzyz() const;\
	vector_<ScalarT,4> yzzx() const;\
	vector_<ScalarT,4> yzzy() const;\
	vector_<ScalarT,4> yzzz() const;\
	vector_<ScalarT,4> zxxx() const;\
	vector_<ScalarT,4> zxxy() const;\
	vector_<ScalarT,4> zxxz() const;\
	vector_<ScalarT,4> zxyx() const;\
	vector_<ScalarT,4> zxyy() const;\
	vector_<ScalarT,4> zxyz() const;\
	vector_<ScalarT,4> zxzx() const;\
	vector_<ScalarT,4> zxzy() const;\
	vector_<ScalarT,4> zxzz() const;\
	vector_<ScalarT,4> zyxx() const;\
	vector_<ScalarT,4> zyxy() const;\
	vector_<ScalarT,4> zyxz() const;\
	vector_<ScalarT,4> zyyx() const;\
	vector_<ScalarT,4> zyyy() const;\
	vector_<ScalarT,4> zyyz() const;\
	vector_<ScalarT,4> zyzx() const;\
	vector_<ScalarT,4> zyzy() const;\
	vector_<ScalarT,4> zyzz() const;\
	vector_<ScalarT,4> zzxx() const;\
	vector_<ScalarT,4> zzxy() const;\
	vector_<ScalarT,4> zzxz() const;\
	vector_<ScalarT,4> zzyx() const;\
	vector_<ScalarT,4> zzyy() const;\
	vector_<ScalarT,4> zzyz() const;\
	vector_<ScalarT,4> zzzx() const;\
	vector_<ScalarT,4> zzzy() const;\
	vector_<ScalarT,4> zzzz() const;\
	ScalarT const& w() const;\
	ScalarT& w();\
	vector_<ScalarT,2> xw() const;\
	vector_<ScalarT,2> yw() const;\
	vector_<ScalarT,2> zw() const;\
	vector_<ScalarT,2> wx() const;\
	vector_<ScalarT,2> wy() const;\
	vector_<ScalarT,2> wz() const;\
	vector_<ScalarT,2> ww() const;\
	vector_<ScalarT,3> xxw() const;\
	vector_<ScalarT,3> xyw() const;\
	vector_<ScalarT,3> xzw() const;\
	vector_<ScalarT,3> xwx() const;\
	vector_<ScalarT,3> xwy() const;\
	vector_<ScalarT,3> xwz() const;\
	vector_<ScalarT,3> xww() const;\
	vector_<ScalarT,3> yxw() const;\
	vector_<ScalarT,3> yyw() const;\
	vector_<ScalarT,3> yzw() const;\
	vector_<ScalarT,3> ywx() const;\
	vector_<ScalarT,3> ywy() const;\
	vector_<ScalarT,3> ywz() const;\
	vector_<ScalarT,3> yww() const;\
	vector_<ScalarT,3> zxw() const;\
	vector_<ScalarT,3> zyw() const;\
	vector_<ScalarT,3> zzw() const;\
	vector_<ScalarT,3> zwx() const;\
	vector_<ScalarT,3> zwy() const;\
	vector_<ScalarT,3> zwz() const;\
	vector_<ScalarT,3> zww() const;\
	vector_<ScalarT,3> wxx() const;\
	vector_<ScalarT,3> wxy() const;\
	vector_<ScalarT,3> wxz() const;\
	vector_<ScalarT,3> wxw() const;\
	vector_<ScalarT,3> wyx() const;\
	vector_<ScalarT,3> wyy() const;\
	vector_<ScalarT,3> wyz() const;\
	vector_<ScalarT,3> wyw() const;\
	vector_<ScalarT,3> wzx() const;\
	vector_<ScalarT,3> wzy() const;\
	vector_<ScalarT,3> wzz() const;\
	vector_<ScalarT,3> wzw() const;\
	vector_<ScalarT,3> wwx() const;\
	vector_<ScalarT,3> wwy() const;\
	vector_<ScalarT,3> wwz() const;\
	vector_<ScalarT,3> www() const;\
	vector_<ScalarT,4> xxxw() const;\
	vector_<ScalarT,4> xxyw() const;\
	vector_<ScalarT,4> xxzw() const;\
	vector_<ScalarT,4> xxwx() const;\
	vector_<ScalarT,4> xxwy() const;\
	vector_<ScalarT,4> xxwz() const;\
	vector_<ScalarT,4> xxww() const;\
	vector_<ScalarT,4> xyxw() const;\
	vector_<ScalarT,4> xyyw() const;\
	vector_<ScalarT,4> xyzw() const;\
	vector_<ScalarT,4> xywx() const;\
	vector_<ScalarT,4> xywy() const;\
	vector_<ScalarT,4> xywz() const;\
	vector_<ScalarT,4> xyww() const;\
	vector_<ScalarT,4> xzxw() const;\
	vector_<ScalarT,4> xzyw() const;\
	vector_<ScalarT,4> xzzw() const;\
	vector_<ScalarT,4> xzwx() const;\
	vector_<ScalarT,4> xzwy() const;\
	vector_<ScalarT,4> xzwz() const;\
	vector_<ScalarT,4> xzww() const;\
	vector_<ScalarT,4> xwxx() const;\
	vector_<ScalarT,4> xwxy() const;\
	vector_<ScalarT,4> xwxz() const;\
	vector_<ScalarT,4> xwxw() const;\
	vector_<ScalarT,4> xwyx() const;\
	vector_<ScalarT,4> xwyy() const;\
	vector_<ScalarT,4> xwyz() const;\
	vector_<ScalarT,4> xwyw() const;\
	vector_<ScalarT,4> xwzx() const;\
	vector_<ScalarT,4> xwzy() const;\
	vector_<ScalarT,4> xwzz() const;\
	vector_<ScalarT,4> xwzw() const;\
	vector_<ScalarT,4> xwwx() const;\
	vector_<ScalarT,4> xwwy() const;\
	vector_<ScalarT,4> xwwz() const;\
	vector_<ScalarT,4> xwww() const;\
	vector_<ScalarT,4> yxxw() const;\
	vector_<ScalarT,4> yxyw() const;\
	vector_<ScalarT,4> yxzw() const;\
	vector_<ScalarT,4> yxwx() const;\
	vector_<ScalarT,4> yxwy() const;\
	vector_<ScalarT,4> yxwz() const;\
	vector_<ScalarT,4> yxww() const;\
	vector_<ScalarT,4> yyxw() const;\
	vector_<ScalarT,4> yyyw() const;\
	vector_<ScalarT,4> yyzw() const;\
	vector_<ScalarT,4> yywx() const;\
	vector_<ScalarT,4> yywy() const;\
	vector_<ScalarT,4> yywz() const;\
	vector_<ScalarT,4> yyww() const;\
	vector_<ScalarT,4> yzxw() const;\
	vector_<ScalarT,4> yzyw() const;\
	vector_<ScalarT,4> yzzw() const;\
	vector_<ScalarT,4> yzwx() const;\
	vector_<ScalarT,4> yzwy() const;\
	vector_<ScalarT,4> yzwz() const;\
	vector_<ScalarT,4> yzww() const;\
	vector_<ScalarT,4> ywxx() const;\
	vector_<ScalarT,4> ywxy() const;\
	vector_<ScalarT,4> ywxz() const;\
	vector_<ScalarT,4> ywxw() const;\
	vector_<ScalarT,4> ywyx() const;\
	vector_<ScalarT,4> ywyy() const;\
	vector_<ScalarT,4> ywyz() const;\
	vector_<ScalarT,4> ywyw() const;\
	vector_<ScalarT,4> ywzx() const;\
	vector_<ScalarT,4> ywzy() const;\
	vector_<ScalarT,4> ywzz() const;\
	vector_<ScalarT,4> ywzw() const;\
	vector_<ScalarT,4> ywwx() const;\
	vector_<ScalarT,4> ywwy() const;\
	vector_<ScalarT,4> ywwz() const;\
	vector_<ScalarT,4> ywww() const;\
	vector_<ScalarT,4> zxxw() const;\
	vector_<ScalarT,4> zxyw() const;\
	vector_<ScalarT,4> zxzw() const;\
	vector_<ScalarT,4> zxwx() const;\
	vector_<ScalarT,4> zxwy() const;\
	vector_<ScalarT,4> zxwz() const;\
	vector_<ScalarT,4> zxww() const;\
	vector_<ScalarT,4> zyxw() const;\
	vector_<ScalarT,4> zyyw() const;\
	vector_<ScalarT,4> zyzw() const;\
	vector_<ScalarT,4> zywx() const;\
	vector_<ScalarT,4> zywy() const;\
	vector_<ScalarT,4> zywz() const;\
	vector_<ScalarT,4> zyww() const;\
	vector_<ScalarT,4> zzxw() const;\
	vector_<ScalarT,4> zzyw() const;\
	vector_<ScalarT,4> zzzw() const;\
	vector_<ScalarT,4> zzwx() const;\
	vector_<ScalarT,4> zzwy() const;\
	vector_<ScalarT,4> zzwz() const;\
	vector_<ScalarT,4> zzww() const;\
	vector_<ScalarT,4> zwxx() const;\
	vector_<ScalarT,4> zwxy() const;\
	vector_<ScalarT,4> zwxz() const;\
	vector_<ScalarT,4> zwxw() const;\
	vector_<ScalarT,4> zwyx() const;\
	vector_<ScalarT,4> zwyy() const;\
	vector_<ScalarT,4> zwyz() const;\
	vector_<ScalarT,4> zwyw() const;\
	vector_<ScalarT,4> zwzx() const;\
	vector_<ScalarT,4> zwzy() const;\
	vector_<ScalarT,4> zwzz() const;\
	vector_<ScalarT,4> zwzw() const;\
	vector_<ScalarT,4> zwwx() const;\
	vector_<ScalarT,4> zwwy() const;\
	vector_<ScalarT,4> zwwz() const;\
	vector_<ScalarT,4> zwww() const;\
	vector_<ScalarT,4> wxxx() const;\
	vector_<ScalarT,4> wxxy() const;\
	vector_<ScalarT,4> wxxz() const;\
	vector_<ScalarT,4> wxxw() const;\
	vector_<ScalarT,4> wxyx() const;\
	vector_<ScalarT,4> wxyy() const;\
	vector_<ScalarT,4> wxyz() const;\
	vector_<ScalarT,4> wxyw() const;\
	vector_<ScalarT,4> wxzx() const;\
	vector_<ScalarT,4> wxzy() const;\
	vector_<ScalarT,4> wxzz() const;\
	vector_<ScalarT,4> wxzw() const;\
	vector_<ScalarT,4> wxwx() const;\
	vector_<ScalarT,4> wxwy() const;\
	vector_<ScalarT,4> wxwz() const;\
	vector_<ScalarT,4> wxww() const;\
	vector_<ScalarT,4> wyxx() const;\
	vector_<ScalarT,4> wyxy() const;\
	vector_<ScalarT,4> wyxz() const;\
	vector_<ScalarT,4> wyxw() const;\
	vector_<ScalarT,4> wyyx() const;\
	vector_<ScalarT,4> wyyy() const;\
	vector_<ScalarT,4> wyyz() const;\
	vector_<ScalarT,4> wyyw() const;\
	vector_<ScalarT,4> wyzx() const;\
	vector_<ScalarT,4> wyzy() const;\
	vector_<ScalarT,4> wyzz() const;\
	vector_<ScalarT,4> wyzw() const;\
	vector_<ScalarT,4> wywx() const;\
	vector_<ScalarT,4> wywy() const;\
	vector_<ScalarT,4> wywz() const;\
	vector_<ScalarT,4> wyww() const;\
	vector_<ScalarT,4> wzxx() const;\
	vector_<ScalarT,4> wzxy() const;\
	vector_<ScalarT,4> wzxz() const;\
	vector_<ScalarT,4> wzxw() const;\
	vector_<ScalarT,4> wzyx() const;\
	vector_<ScalarT,4> wzyy() const;\
	vector_<ScalarT,4> wzyz() const;\
	vector_<ScalarT,4> wzyw() const;\
	vector_<ScalarT,4> wzzx() const;\
	vector_<ScalarT,4> wzzy() const;\
	vector_<ScalarT,4> wzzz() const;\
	vector_<ScalarT,4> wzzw() const;\
	vector_<ScalarT,4> wzwx() const;\
	vector_<ScalarT,4> wzwy() const;\
	vector_<ScalarT,4> wzwz() const;\
	vector_<ScalarT,4> wzww() const;\
	vector_<ScalarT,4> wwxx() const;\
	vector_<ScalarT,4> wwxy() const;\
	vector_<ScalarT,4> wwxz() const;\
	vector_<ScalarT,4> wwxw() const;\
	vector_<ScalarT,4> wwyx() const;\
	vector_<ScalarT,4> wwyy() const;\
	vector_<ScalarT,4> wwyz() const;\
	vector_<ScalarT,4> wwyw() const;\
	vector_<ScalarT,4> wwzx() const;\
	vector_<ScalarT,4> wwzy() const;\
	vector_<ScalarT,4> wwzz() const;\
	vector_<ScalarT,4> wwzw() const;\
	vector_<ScalarT,4> wwwx() const;\
	vector_<ScalarT,4> wwwy() const;\
	vector_<ScalarT,4> wwwz() const;\
	vector_<ScalarT,4> wwww() const;\
	;

#define SWIZZLE_IMPL_FOR_VEC2() \
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,2>::x() const{\
		return ((ScalarT const*)(this))[0];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,2>::x(){\
		return ((ScalarT*)(this))[0];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,2>::xx() const{\
		return vector_<ScalarT,2>(x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::xxx() const{\
		return vector_<ScalarT,3>(x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xxxx() const{\
		return vector_<ScalarT,4>(x(), x(), x(), x());\
	}\
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,2>::y() const{\
		return ((ScalarT const*)(this))[1];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,2>::y(){\
		return ((ScalarT*)(this))[1];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,2>::xy() const{\
		return vector_<ScalarT,2>(x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,2>::yx() const{\
		return vector_<ScalarT,2>(y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,2>::yy() const{\
		return vector_<ScalarT,2>(y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::xxy() const{\
		return vector_<ScalarT,3>(x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::xyx() const{\
		return vector_<ScalarT,3>(x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::xyy() const{\
		return vector_<ScalarT,3>(x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::yxx() const{\
		return vector_<ScalarT,3>(y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::yxy() const{\
		return vector_<ScalarT,3>(y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::yyx() const{\
		return vector_<ScalarT,3>(y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,2>::yyy() const{\
		return vector_<ScalarT,3>(y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xxxy() const{\
		return vector_<ScalarT,4>(x(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xxyx() const{\
		return vector_<ScalarT,4>(x(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xxyy() const{\
		return vector_<ScalarT,4>(x(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xyxx() const{\
		return vector_<ScalarT,4>(x(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xyxy() const{\
		return vector_<ScalarT,4>(x(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xyyx() const{\
		return vector_<ScalarT,4>(x(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::xyyy() const{\
		return vector_<ScalarT,4>(x(), y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yxxx() const{\
		return vector_<ScalarT,4>(y(), x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yxxy() const{\
		return vector_<ScalarT,4>(y(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yxyx() const{\
		return vector_<ScalarT,4>(y(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yxyy() const{\
		return vector_<ScalarT,4>(y(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yyxx() const{\
		return vector_<ScalarT,4>(y(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yyxy() const{\
		return vector_<ScalarT,4>(y(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yyyx() const{\
		return vector_<ScalarT,4>(y(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,2>::yyyy() const{\
		return vector_<ScalarT,4>(y(), y(), y(), y());\
	}\
	;

#define SWIZZLE_IMPL_FOR_VEC3() \
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,3>::x() const{\
		return ((ScalarT const*)(this))[0];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,3>::x(){\
		return ((ScalarT*)(this))[0];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::xx() const{\
		return vector_<ScalarT,2>(x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xxx() const{\
		return vector_<ScalarT,3>(x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxxx() const{\
		return vector_<ScalarT,4>(x(), x(), x(), x());\
	}\
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,3>::y() const{\
		return ((ScalarT const*)(this))[1];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,3>::y(){\
		return ((ScalarT*)(this))[1];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::xy() const{\
		return vector_<ScalarT,2>(x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::yx() const{\
		return vector_<ScalarT,2>(y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::yy() const{\
		return vector_<ScalarT,2>(y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xxy() const{\
		return vector_<ScalarT,3>(x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xyx() const{\
		return vector_<ScalarT,3>(x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xyy() const{\
		return vector_<ScalarT,3>(x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yxx() const{\
		return vector_<ScalarT,3>(y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yxy() const{\
		return vector_<ScalarT,3>(y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yyx() const{\
		return vector_<ScalarT,3>(y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yyy() const{\
		return vector_<ScalarT,3>(y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxxy() const{\
		return vector_<ScalarT,4>(x(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxyx() const{\
		return vector_<ScalarT,4>(x(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxyy() const{\
		return vector_<ScalarT,4>(x(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyxx() const{\
		return vector_<ScalarT,4>(x(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyxy() const{\
		return vector_<ScalarT,4>(x(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyyx() const{\
		return vector_<ScalarT,4>(x(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyyy() const{\
		return vector_<ScalarT,4>(x(), y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxxx() const{\
		return vector_<ScalarT,4>(y(), x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxxy() const{\
		return vector_<ScalarT,4>(y(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxyx() const{\
		return vector_<ScalarT,4>(y(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxyy() const{\
		return vector_<ScalarT,4>(y(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyxx() const{\
		return vector_<ScalarT,4>(y(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyxy() const{\
		return vector_<ScalarT,4>(y(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyyx() const{\
		return vector_<ScalarT,4>(y(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyyy() const{\
		return vector_<ScalarT,4>(y(), y(), y(), y());\
	}\
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,3>::z() const{\
		return ((ScalarT const*)(this))[2];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,3>::z(){\
		return ((ScalarT*)(this))[2];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::xz() const{\
		return vector_<ScalarT,2>(x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::yz() const{\
		return vector_<ScalarT,2>(y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::zx() const{\
		return vector_<ScalarT,2>(z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::zy() const{\
		return vector_<ScalarT,2>(z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,3>::zz() const{\
		return vector_<ScalarT,2>(z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xxz() const{\
		return vector_<ScalarT,3>(x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xyz() const{\
		return vector_<ScalarT,3>(x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xzx() const{\
		return vector_<ScalarT,3>(x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xzy() const{\
		return vector_<ScalarT,3>(x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::xzz() const{\
		return vector_<ScalarT,3>(x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yxz() const{\
		return vector_<ScalarT,3>(y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yyz() const{\
		return vector_<ScalarT,3>(y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yzx() const{\
		return vector_<ScalarT,3>(y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yzy() const{\
		return vector_<ScalarT,3>(y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::yzz() const{\
		return vector_<ScalarT,3>(y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zxx() const{\
		return vector_<ScalarT,3>(z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zxy() const{\
		return vector_<ScalarT,3>(z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zxz() const{\
		return vector_<ScalarT,3>(z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zyx() const{\
		return vector_<ScalarT,3>(z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zyy() const{\
		return vector_<ScalarT,3>(z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zyz() const{\
		return vector_<ScalarT,3>(z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zzx() const{\
		return vector_<ScalarT,3>(z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zzy() const{\
		return vector_<ScalarT,3>(z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,3>::zzz() const{\
		return vector_<ScalarT,3>(z(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxxz() const{\
		return vector_<ScalarT,4>(x(), x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxyz() const{\
		return vector_<ScalarT,4>(x(), x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxzx() const{\
		return vector_<ScalarT,4>(x(), x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxzy() const{\
		return vector_<ScalarT,4>(x(), x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xxzz() const{\
		return vector_<ScalarT,4>(x(), x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyxz() const{\
		return vector_<ScalarT,4>(x(), y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyyz() const{\
		return vector_<ScalarT,4>(x(), y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyzx() const{\
		return vector_<ScalarT,4>(x(), y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyzy() const{\
		return vector_<ScalarT,4>(x(), y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xyzz() const{\
		return vector_<ScalarT,4>(x(), y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzxx() const{\
		return vector_<ScalarT,4>(x(), z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzxy() const{\
		return vector_<ScalarT,4>(x(), z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzxz() const{\
		return vector_<ScalarT,4>(x(), z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzyx() const{\
		return vector_<ScalarT,4>(x(), z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzyy() const{\
		return vector_<ScalarT,4>(x(), z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzyz() const{\
		return vector_<ScalarT,4>(x(), z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzzx() const{\
		return vector_<ScalarT,4>(x(), z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzzy() const{\
		return vector_<ScalarT,4>(x(), z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::xzzz() const{\
		return vector_<ScalarT,4>(x(), z(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxxz() const{\
		return vector_<ScalarT,4>(y(), x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxyz() const{\
		return vector_<ScalarT,4>(y(), x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxzx() const{\
		return vector_<ScalarT,4>(y(), x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxzy() const{\
		return vector_<ScalarT,4>(y(), x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yxzz() const{\
		return vector_<ScalarT,4>(y(), x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyxz() const{\
		return vector_<ScalarT,4>(y(), y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyyz() const{\
		return vector_<ScalarT,4>(y(), y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyzx() const{\
		return vector_<ScalarT,4>(y(), y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyzy() const{\
		return vector_<ScalarT,4>(y(), y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yyzz() const{\
		return vector_<ScalarT,4>(y(), y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzxx() const{\
		return vector_<ScalarT,4>(y(), z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzxy() const{\
		return vector_<ScalarT,4>(y(), z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzxz() const{\
		return vector_<ScalarT,4>(y(), z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzyx() const{\
		return vector_<ScalarT,4>(y(), z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzyy() const{\
		return vector_<ScalarT,4>(y(), z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzyz() const{\
		return vector_<ScalarT,4>(y(), z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzzx() const{\
		return vector_<ScalarT,4>(y(), z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzzy() const{\
		return vector_<ScalarT,4>(y(), z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::yzzz() const{\
		return vector_<ScalarT,4>(y(), z(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxxx() const{\
		return vector_<ScalarT,4>(z(), x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxxy() const{\
		return vector_<ScalarT,4>(z(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxxz() const{\
		return vector_<ScalarT,4>(z(), x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxyx() const{\
		return vector_<ScalarT,4>(z(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxyy() const{\
		return vector_<ScalarT,4>(z(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxyz() const{\
		return vector_<ScalarT,4>(z(), x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxzx() const{\
		return vector_<ScalarT,4>(z(), x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxzy() const{\
		return vector_<ScalarT,4>(z(), x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zxzz() const{\
		return vector_<ScalarT,4>(z(), x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyxx() const{\
		return vector_<ScalarT,4>(z(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyxy() const{\
		return vector_<ScalarT,4>(z(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyxz() const{\
		return vector_<ScalarT,4>(z(), y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyyx() const{\
		return vector_<ScalarT,4>(z(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyyy() const{\
		return vector_<ScalarT,4>(z(), y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyyz() const{\
		return vector_<ScalarT,4>(z(), y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyzx() const{\
		return vector_<ScalarT,4>(z(), y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyzy() const{\
		return vector_<ScalarT,4>(z(), y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zyzz() const{\
		return vector_<ScalarT,4>(z(), y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzxx() const{\
		return vector_<ScalarT,4>(z(), z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzxy() const{\
		return vector_<ScalarT,4>(z(), z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzxz() const{\
		return vector_<ScalarT,4>(z(), z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzyx() const{\
		return vector_<ScalarT,4>(z(), z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzyy() const{\
		return vector_<ScalarT,4>(z(), z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzyz() const{\
		return vector_<ScalarT,4>(z(), z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzzx() const{\
		return vector_<ScalarT,4>(z(), z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzzy() const{\
		return vector_<ScalarT,4>(z(), z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,3>::zzzz() const{\
		return vector_<ScalarT,4>(z(), z(), z(), z());\
	}\
	;

#define SWIZZLE_IMPL_FOR_VEC4() \
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,4>::x() const{\
		return ((ScalarT const*)(this))[0];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,4>::x(){\
		return ((ScalarT*)(this))[0];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::xx() const{\
		return vector_<ScalarT,2>(x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xxx() const{\
		return vector_<ScalarT,3>(x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxxx() const{\
		return vector_<ScalarT,4>(x(), x(), x(), x());\
	}\
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,4>::y() const{\
		return ((ScalarT const*)(this))[1];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,4>::y(){\
		return ((ScalarT*)(this))[1];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::xy() const{\
		return vector_<ScalarT,2>(x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::yx() const{\
		return vector_<ScalarT,2>(y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::yy() const{\
		return vector_<ScalarT,2>(y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xxy() const{\
		return vector_<ScalarT,3>(x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xyx() const{\
		return vector_<ScalarT,3>(x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xyy() const{\
		return vector_<ScalarT,3>(x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yxx() const{\
		return vector_<ScalarT,3>(y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yxy() const{\
		return vector_<ScalarT,3>(y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yyx() const{\
		return vector_<ScalarT,3>(y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yyy() const{\
		return vector_<ScalarT,3>(y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxxy() const{\
		return vector_<ScalarT,4>(x(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxyx() const{\
		return vector_<ScalarT,4>(x(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxyy() const{\
		return vector_<ScalarT,4>(x(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyxx() const{\
		return vector_<ScalarT,4>(x(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyxy() const{\
		return vector_<ScalarT,4>(x(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyyx() const{\
		return vector_<ScalarT,4>(x(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyyy() const{\
		return vector_<ScalarT,4>(x(), y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxxx() const{\
		return vector_<ScalarT,4>(y(), x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxxy() const{\
		return vector_<ScalarT,4>(y(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxyx() const{\
		return vector_<ScalarT,4>(y(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxyy() const{\
		return vector_<ScalarT,4>(y(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyxx() const{\
		return vector_<ScalarT,4>(y(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyxy() const{\
		return vector_<ScalarT,4>(y(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyyx() const{\
		return vector_<ScalarT,4>(y(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyyy() const{\
		return vector_<ScalarT,4>(y(), y(), y(), y());\
	}\
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,4>::z() const{\
		return ((ScalarT const*)(this))[2];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,4>::z(){\
		return ((ScalarT*)(this))[2];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::xz() const{\
		return vector_<ScalarT,2>(x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::yz() const{\
		return vector_<ScalarT,2>(y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::zx() const{\
		return vector_<ScalarT,2>(z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::zy() const{\
		return vector_<ScalarT,2>(z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::zz() const{\
		return vector_<ScalarT,2>(z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xxz() const{\
		return vector_<ScalarT,3>(x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xyz() const{\
		return vector_<ScalarT,3>(x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xzx() const{\
		return vector_<ScalarT,3>(x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xzy() const{\
		return vector_<ScalarT,3>(x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xzz() const{\
		return vector_<ScalarT,3>(x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yxz() const{\
		return vector_<ScalarT,3>(y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yyz() const{\
		return vector_<ScalarT,3>(y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yzx() const{\
		return vector_<ScalarT,3>(y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yzy() const{\
		return vector_<ScalarT,3>(y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yzz() const{\
		return vector_<ScalarT,3>(y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zxx() const{\
		return vector_<ScalarT,3>(z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zxy() const{\
		return vector_<ScalarT,3>(z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zxz() const{\
		return vector_<ScalarT,3>(z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zyx() const{\
		return vector_<ScalarT,3>(z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zyy() const{\
		return vector_<ScalarT,3>(z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zyz() const{\
		return vector_<ScalarT,3>(z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zzx() const{\
		return vector_<ScalarT,3>(z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zzy() const{\
		return vector_<ScalarT,3>(z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zzz() const{\
		return vector_<ScalarT,3>(z(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxxz() const{\
		return vector_<ScalarT,4>(x(), x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxyz() const{\
		return vector_<ScalarT,4>(x(), x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxzx() const{\
		return vector_<ScalarT,4>(x(), x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxzy() const{\
		return vector_<ScalarT,4>(x(), x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxzz() const{\
		return vector_<ScalarT,4>(x(), x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyxz() const{\
		return vector_<ScalarT,4>(x(), y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyyz() const{\
		return vector_<ScalarT,4>(x(), y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyzx() const{\
		return vector_<ScalarT,4>(x(), y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyzy() const{\
		return vector_<ScalarT,4>(x(), y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyzz() const{\
		return vector_<ScalarT,4>(x(), y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzxx() const{\
		return vector_<ScalarT,4>(x(), z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzxy() const{\
		return vector_<ScalarT,4>(x(), z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzxz() const{\
		return vector_<ScalarT,4>(x(), z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzyx() const{\
		return vector_<ScalarT,4>(x(), z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzyy() const{\
		return vector_<ScalarT,4>(x(), z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzyz() const{\
		return vector_<ScalarT,4>(x(), z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzzx() const{\
		return vector_<ScalarT,4>(x(), z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzzy() const{\
		return vector_<ScalarT,4>(x(), z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzzz() const{\
		return vector_<ScalarT,4>(x(), z(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxxz() const{\
		return vector_<ScalarT,4>(y(), x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxyz() const{\
		return vector_<ScalarT,4>(y(), x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxzx() const{\
		return vector_<ScalarT,4>(y(), x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxzy() const{\
		return vector_<ScalarT,4>(y(), x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxzz() const{\
		return vector_<ScalarT,4>(y(), x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyxz() const{\
		return vector_<ScalarT,4>(y(), y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyyz() const{\
		return vector_<ScalarT,4>(y(), y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyzx() const{\
		return vector_<ScalarT,4>(y(), y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyzy() const{\
		return vector_<ScalarT,4>(y(), y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyzz() const{\
		return vector_<ScalarT,4>(y(), y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzxx() const{\
		return vector_<ScalarT,4>(y(), z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzxy() const{\
		return vector_<ScalarT,4>(y(), z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzxz() const{\
		return vector_<ScalarT,4>(y(), z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzyx() const{\
		return vector_<ScalarT,4>(y(), z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzyy() const{\
		return vector_<ScalarT,4>(y(), z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzyz() const{\
		return vector_<ScalarT,4>(y(), z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzzx() const{\
		return vector_<ScalarT,4>(y(), z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzzy() const{\
		return vector_<ScalarT,4>(y(), z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzzz() const{\
		return vector_<ScalarT,4>(y(), z(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxxx() const{\
		return vector_<ScalarT,4>(z(), x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxxy() const{\
		return vector_<ScalarT,4>(z(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxxz() const{\
		return vector_<ScalarT,4>(z(), x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxyx() const{\
		return vector_<ScalarT,4>(z(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxyy() const{\
		return vector_<ScalarT,4>(z(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxyz() const{\
		return vector_<ScalarT,4>(z(), x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxzx() const{\
		return vector_<ScalarT,4>(z(), x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxzy() const{\
		return vector_<ScalarT,4>(z(), x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxzz() const{\
		return vector_<ScalarT,4>(z(), x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyxx() const{\
		return vector_<ScalarT,4>(z(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyxy() const{\
		return vector_<ScalarT,4>(z(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyxz() const{\
		return vector_<ScalarT,4>(z(), y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyyx() const{\
		return vector_<ScalarT,4>(z(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyyy() const{\
		return vector_<ScalarT,4>(z(), y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyyz() const{\
		return vector_<ScalarT,4>(z(), y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyzx() const{\
		return vector_<ScalarT,4>(z(), y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyzy() const{\
		return vector_<ScalarT,4>(z(), y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyzz() const{\
		return vector_<ScalarT,4>(z(), y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzxx() const{\
		return vector_<ScalarT,4>(z(), z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzxy() const{\
		return vector_<ScalarT,4>(z(), z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzxz() const{\
		return vector_<ScalarT,4>(z(), z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzyx() const{\
		return vector_<ScalarT,4>(z(), z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzyy() const{\
		return vector_<ScalarT,4>(z(), z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzyz() const{\
		return vector_<ScalarT,4>(z(), z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzzx() const{\
		return vector_<ScalarT,4>(z(), z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzzy() const{\
		return vector_<ScalarT,4>(z(), z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzzz() const{\
		return vector_<ScalarT,4>(z(), z(), z(), z());\
	}\
	template<typename ScalarT> ScalarT const& vector_swizzle<ScalarT,4>::w() const{\
		return ((ScalarT const*)(this))[3];\
	}\
	template<typename ScalarT> ScalarT& vector_swizzle<ScalarT,4>::w(){\
		return ((ScalarT*)(this))[3];\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::xw() const{\
		return vector_<ScalarT,2>(x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::yw() const{\
		return vector_<ScalarT,2>(y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::zw() const{\
		return vector_<ScalarT,2>(z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::wx() const{\
		return vector_<ScalarT,2>(w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::wy() const{\
		return vector_<ScalarT,2>(w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::wz() const{\
		return vector_<ScalarT,2>(w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,2> vector_swizzle<ScalarT,4>::ww() const{\
		return vector_<ScalarT,2>(w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xxw() const{\
		return vector_<ScalarT,3>(x(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xyw() const{\
		return vector_<ScalarT,3>(x(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xzw() const{\
		return vector_<ScalarT,3>(x(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xwx() const{\
		return vector_<ScalarT,3>(x(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xwy() const{\
		return vector_<ScalarT,3>(x(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xwz() const{\
		return vector_<ScalarT,3>(x(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::xww() const{\
		return vector_<ScalarT,3>(x(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yxw() const{\
		return vector_<ScalarT,3>(y(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yyw() const{\
		return vector_<ScalarT,3>(y(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yzw() const{\
		return vector_<ScalarT,3>(y(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::ywx() const{\
		return vector_<ScalarT,3>(y(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::ywy() const{\
		return vector_<ScalarT,3>(y(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::ywz() const{\
		return vector_<ScalarT,3>(y(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::yww() const{\
		return vector_<ScalarT,3>(y(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zxw() const{\
		return vector_<ScalarT,3>(z(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zyw() const{\
		return vector_<ScalarT,3>(z(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zzw() const{\
		return vector_<ScalarT,3>(z(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zwx() const{\
		return vector_<ScalarT,3>(z(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zwy() const{\
		return vector_<ScalarT,3>(z(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zwz() const{\
		return vector_<ScalarT,3>(z(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::zww() const{\
		return vector_<ScalarT,3>(z(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wxx() const{\
		return vector_<ScalarT,3>(w(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wxy() const{\
		return vector_<ScalarT,3>(w(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wxz() const{\
		return vector_<ScalarT,3>(w(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wxw() const{\
		return vector_<ScalarT,3>(w(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wyx() const{\
		return vector_<ScalarT,3>(w(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wyy() const{\
		return vector_<ScalarT,3>(w(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wyz() const{\
		return vector_<ScalarT,3>(w(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wyw() const{\
		return vector_<ScalarT,3>(w(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wzx() const{\
		return vector_<ScalarT,3>(w(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wzy() const{\
		return vector_<ScalarT,3>(w(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wzz() const{\
		return vector_<ScalarT,3>(w(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wzw() const{\
		return vector_<ScalarT,3>(w(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wwx() const{\
		return vector_<ScalarT,3>(w(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wwy() const{\
		return vector_<ScalarT,3>(w(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::wwz() const{\
		return vector_<ScalarT,3>(w(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,3> vector_swizzle<ScalarT,4>::www() const{\
		return vector_<ScalarT,3>(w(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxxw() const{\
		return vector_<ScalarT,4>(x(), x(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxyw() const{\
		return vector_<ScalarT,4>(x(), x(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxzw() const{\
		return vector_<ScalarT,4>(x(), x(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxwx() const{\
		return vector_<ScalarT,4>(x(), x(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxwy() const{\
		return vector_<ScalarT,4>(x(), x(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxwz() const{\
		return vector_<ScalarT,4>(x(), x(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xxww() const{\
		return vector_<ScalarT,4>(x(), x(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyxw() const{\
		return vector_<ScalarT,4>(x(), y(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyyw() const{\
		return vector_<ScalarT,4>(x(), y(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyzw() const{\
		return vector_<ScalarT,4>(x(), y(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xywx() const{\
		return vector_<ScalarT,4>(x(), y(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xywy() const{\
		return vector_<ScalarT,4>(x(), y(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xywz() const{\
		return vector_<ScalarT,4>(x(), y(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xyww() const{\
		return vector_<ScalarT,4>(x(), y(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzxw() const{\
		return vector_<ScalarT,4>(x(), z(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzyw() const{\
		return vector_<ScalarT,4>(x(), z(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzzw() const{\
		return vector_<ScalarT,4>(x(), z(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzwx() const{\
		return vector_<ScalarT,4>(x(), z(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzwy() const{\
		return vector_<ScalarT,4>(x(), z(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzwz() const{\
		return vector_<ScalarT,4>(x(), z(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xzww() const{\
		return vector_<ScalarT,4>(x(), z(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwxx() const{\
		return vector_<ScalarT,4>(x(), w(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwxy() const{\
		return vector_<ScalarT,4>(x(), w(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwxz() const{\
		return vector_<ScalarT,4>(x(), w(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwxw() const{\
		return vector_<ScalarT,4>(x(), w(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwyx() const{\
		return vector_<ScalarT,4>(x(), w(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwyy() const{\
		return vector_<ScalarT,4>(x(), w(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwyz() const{\
		return vector_<ScalarT,4>(x(), w(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwyw() const{\
		return vector_<ScalarT,4>(x(), w(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwzx() const{\
		return vector_<ScalarT,4>(x(), w(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwzy() const{\
		return vector_<ScalarT,4>(x(), w(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwzz() const{\
		return vector_<ScalarT,4>(x(), w(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwzw() const{\
		return vector_<ScalarT,4>(x(), w(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwwx() const{\
		return vector_<ScalarT,4>(x(), w(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwwy() const{\
		return vector_<ScalarT,4>(x(), w(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwwz() const{\
		return vector_<ScalarT,4>(x(), w(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::xwww() const{\
		return vector_<ScalarT,4>(x(), w(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxxw() const{\
		return vector_<ScalarT,4>(y(), x(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxyw() const{\
		return vector_<ScalarT,4>(y(), x(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxzw() const{\
		return vector_<ScalarT,4>(y(), x(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxwx() const{\
		return vector_<ScalarT,4>(y(), x(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxwy() const{\
		return vector_<ScalarT,4>(y(), x(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxwz() const{\
		return vector_<ScalarT,4>(y(), x(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yxww() const{\
		return vector_<ScalarT,4>(y(), x(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyxw() const{\
		return vector_<ScalarT,4>(y(), y(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyyw() const{\
		return vector_<ScalarT,4>(y(), y(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyzw() const{\
		return vector_<ScalarT,4>(y(), y(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yywx() const{\
		return vector_<ScalarT,4>(y(), y(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yywy() const{\
		return vector_<ScalarT,4>(y(), y(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yywz() const{\
		return vector_<ScalarT,4>(y(), y(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yyww() const{\
		return vector_<ScalarT,4>(y(), y(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzxw() const{\
		return vector_<ScalarT,4>(y(), z(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzyw() const{\
		return vector_<ScalarT,4>(y(), z(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzzw() const{\
		return vector_<ScalarT,4>(y(), z(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzwx() const{\
		return vector_<ScalarT,4>(y(), z(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzwy() const{\
		return vector_<ScalarT,4>(y(), z(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzwz() const{\
		return vector_<ScalarT,4>(y(), z(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::yzww() const{\
		return vector_<ScalarT,4>(y(), z(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywxx() const{\
		return vector_<ScalarT,4>(y(), w(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywxy() const{\
		return vector_<ScalarT,4>(y(), w(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywxz() const{\
		return vector_<ScalarT,4>(y(), w(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywxw() const{\
		return vector_<ScalarT,4>(y(), w(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywyx() const{\
		return vector_<ScalarT,4>(y(), w(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywyy() const{\
		return vector_<ScalarT,4>(y(), w(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywyz() const{\
		return vector_<ScalarT,4>(y(), w(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywyw() const{\
		return vector_<ScalarT,4>(y(), w(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywzx() const{\
		return vector_<ScalarT,4>(y(), w(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywzy() const{\
		return vector_<ScalarT,4>(y(), w(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywzz() const{\
		return vector_<ScalarT,4>(y(), w(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywzw() const{\
		return vector_<ScalarT,4>(y(), w(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywwx() const{\
		return vector_<ScalarT,4>(y(), w(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywwy() const{\
		return vector_<ScalarT,4>(y(), w(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywwz() const{\
		return vector_<ScalarT,4>(y(), w(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::ywww() const{\
		return vector_<ScalarT,4>(y(), w(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxxw() const{\
		return vector_<ScalarT,4>(z(), x(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxyw() const{\
		return vector_<ScalarT,4>(z(), x(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxzw() const{\
		return vector_<ScalarT,4>(z(), x(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxwx() const{\
		return vector_<ScalarT,4>(z(), x(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxwy() const{\
		return vector_<ScalarT,4>(z(), x(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxwz() const{\
		return vector_<ScalarT,4>(z(), x(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zxww() const{\
		return vector_<ScalarT,4>(z(), x(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyxw() const{\
		return vector_<ScalarT,4>(z(), y(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyyw() const{\
		return vector_<ScalarT,4>(z(), y(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyzw() const{\
		return vector_<ScalarT,4>(z(), y(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zywx() const{\
		return vector_<ScalarT,4>(z(), y(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zywy() const{\
		return vector_<ScalarT,4>(z(), y(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zywz() const{\
		return vector_<ScalarT,4>(z(), y(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zyww() const{\
		return vector_<ScalarT,4>(z(), y(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzxw() const{\
		return vector_<ScalarT,4>(z(), z(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzyw() const{\
		return vector_<ScalarT,4>(z(), z(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzzw() const{\
		return vector_<ScalarT,4>(z(), z(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzwx() const{\
		return vector_<ScalarT,4>(z(), z(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzwy() const{\
		return vector_<ScalarT,4>(z(), z(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzwz() const{\
		return vector_<ScalarT,4>(z(), z(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zzww() const{\
		return vector_<ScalarT,4>(z(), z(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwxx() const{\
		return vector_<ScalarT,4>(z(), w(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwxy() const{\
		return vector_<ScalarT,4>(z(), w(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwxz() const{\
		return vector_<ScalarT,4>(z(), w(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwxw() const{\
		return vector_<ScalarT,4>(z(), w(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwyx() const{\
		return vector_<ScalarT,4>(z(), w(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwyy() const{\
		return vector_<ScalarT,4>(z(), w(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwyz() const{\
		return vector_<ScalarT,4>(z(), w(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwyw() const{\
		return vector_<ScalarT,4>(z(), w(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwzx() const{\
		return vector_<ScalarT,4>(z(), w(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwzy() const{\
		return vector_<ScalarT,4>(z(), w(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwzz() const{\
		return vector_<ScalarT,4>(z(), w(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwzw() const{\
		return vector_<ScalarT,4>(z(), w(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwwx() const{\
		return vector_<ScalarT,4>(z(), w(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwwy() const{\
		return vector_<ScalarT,4>(z(), w(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwwz() const{\
		return vector_<ScalarT,4>(z(), w(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::zwww() const{\
		return vector_<ScalarT,4>(z(), w(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxxx() const{\
		return vector_<ScalarT,4>(w(), x(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxxy() const{\
		return vector_<ScalarT,4>(w(), x(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxxz() const{\
		return vector_<ScalarT,4>(w(), x(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxxw() const{\
		return vector_<ScalarT,4>(w(), x(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxyx() const{\
		return vector_<ScalarT,4>(w(), x(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxyy() const{\
		return vector_<ScalarT,4>(w(), x(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxyz() const{\
		return vector_<ScalarT,4>(w(), x(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxyw() const{\
		return vector_<ScalarT,4>(w(), x(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxzx() const{\
		return vector_<ScalarT,4>(w(), x(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxzy() const{\
		return vector_<ScalarT,4>(w(), x(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxzz() const{\
		return vector_<ScalarT,4>(w(), x(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxzw() const{\
		return vector_<ScalarT,4>(w(), x(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxwx() const{\
		return vector_<ScalarT,4>(w(), x(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxwy() const{\
		return vector_<ScalarT,4>(w(), x(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxwz() const{\
		return vector_<ScalarT,4>(w(), x(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wxww() const{\
		return vector_<ScalarT,4>(w(), x(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyxx() const{\
		return vector_<ScalarT,4>(w(), y(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyxy() const{\
		return vector_<ScalarT,4>(w(), y(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyxz() const{\
		return vector_<ScalarT,4>(w(), y(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyxw() const{\
		return vector_<ScalarT,4>(w(), y(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyyx() const{\
		return vector_<ScalarT,4>(w(), y(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyyy() const{\
		return vector_<ScalarT,4>(w(), y(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyyz() const{\
		return vector_<ScalarT,4>(w(), y(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyyw() const{\
		return vector_<ScalarT,4>(w(), y(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyzx() const{\
		return vector_<ScalarT,4>(w(), y(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyzy() const{\
		return vector_<ScalarT,4>(w(), y(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyzz() const{\
		return vector_<ScalarT,4>(w(), y(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyzw() const{\
		return vector_<ScalarT,4>(w(), y(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wywx() const{\
		return vector_<ScalarT,4>(w(), y(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wywy() const{\
		return vector_<ScalarT,4>(w(), y(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wywz() const{\
		return vector_<ScalarT,4>(w(), y(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wyww() const{\
		return vector_<ScalarT,4>(w(), y(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzxx() const{\
		return vector_<ScalarT,4>(w(), z(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzxy() const{\
		return vector_<ScalarT,4>(w(), z(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzxz() const{\
		return vector_<ScalarT,4>(w(), z(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzxw() const{\
		return vector_<ScalarT,4>(w(), z(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzyx() const{\
		return vector_<ScalarT,4>(w(), z(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzyy() const{\
		return vector_<ScalarT,4>(w(), z(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzyz() const{\
		return vector_<ScalarT,4>(w(), z(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzyw() const{\
		return vector_<ScalarT,4>(w(), z(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzzx() const{\
		return vector_<ScalarT,4>(w(), z(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzzy() const{\
		return vector_<ScalarT,4>(w(), z(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzzz() const{\
		return vector_<ScalarT,4>(w(), z(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzzw() const{\
		return vector_<ScalarT,4>(w(), z(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzwx() const{\
		return vector_<ScalarT,4>(w(), z(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzwy() const{\
		return vector_<ScalarT,4>(w(), z(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzwz() const{\
		return vector_<ScalarT,4>(w(), z(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wzww() const{\
		return vector_<ScalarT,4>(w(), z(), w(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwxx() const{\
		return vector_<ScalarT,4>(w(), w(), x(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwxy() const{\
		return vector_<ScalarT,4>(w(), w(), x(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwxz() const{\
		return vector_<ScalarT,4>(w(), w(), x(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwxw() const{\
		return vector_<ScalarT,4>(w(), w(), x(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwyx() const{\
		return vector_<ScalarT,4>(w(), w(), y(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwyy() const{\
		return vector_<ScalarT,4>(w(), w(), y(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwyz() const{\
		return vector_<ScalarT,4>(w(), w(), y(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwyw() const{\
		return vector_<ScalarT,4>(w(), w(), y(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwzx() const{\
		return vector_<ScalarT,4>(w(), w(), z(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwzy() const{\
		return vector_<ScalarT,4>(w(), w(), z(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwzz() const{\
		return vector_<ScalarT,4>(w(), w(), z(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwzw() const{\
		return vector_<ScalarT,4>(w(), w(), z(), w());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwwx() const{\
		return vector_<ScalarT,4>(w(), w(), w(), x());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwwy() const{\
		return vector_<ScalarT,4>(w(), w(), w(), y());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwwz() const{\
		return vector_<ScalarT,4>(w(), w(), w(), z());\
	}\
	template<typename ScalarT> vector_<ScalarT,4> vector_swizzle<ScalarT,4>::wwww() const{\
		return vector_<ScalarT,4>(w(), w(), w(), w());\
	}\
	;

#endif