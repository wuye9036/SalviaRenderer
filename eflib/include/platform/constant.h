#ifndef EFLIB_PLATFORM_CONSTANT_H
#define EFLIB_PLATFORM_CONSTANT_H

#include <limits>

namespace eflib{
	static const double	ln10				=	2.30258509299405e+000;
	static const double	invLn10				=	0.43429448190325e+000;
	static const double	PI					=	3.141592653589793238462643383279502884197169399375105820974944592308;	
	static const float	PI_FLOAT			=   float(PI);
	static const double	HALF_PI				=	PI / 2.0;	
	static const double	TWO_PI				=	PI * 2.0;	
	static const double rad_degree			=	57.30;
	static const float 	precision_limit		=	1.0e-15f;
	static const int   	int_max				=	std::numeric_limits< int >::max();
	static const float 	infinite			=	std::numeric_limits< float >::infinity();
	static const float 	epsilon				=	std::numeric_limits< float >::epsilon();
}

#endif
