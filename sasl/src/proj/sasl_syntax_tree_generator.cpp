// sasl_syntax_tree_generator.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "sasl_syntax_tree_generator.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// 这是导出变量的一个示例
SASL_SYNTAX_TREE_GENERATOR_API int nsasl_syntax_tree_generator=0;

// 这是导出函数的一个示例。
SASL_SYNTAX_TREE_GENERATOR_API int fnsasl_syntax_tree_generator(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 sasl_syntax_tree_generator.h
Csasl_syntax_tree_generator::Csasl_syntax_tree_generator()
{
	return;
}
