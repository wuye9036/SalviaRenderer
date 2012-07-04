#ifndef EFLIB_UTILITY_UNREF_DECLARATOR_H
#define EFLIB_UTILITY_UNREF_DECLARATOR_H

template <typename T>
inline void eflib_unref_declarator( T const& /*x*/ ){
	;
}

#define EFLIB_UNREF_DECLARATOR(x) eflib_unref_declarator(x)

#endif