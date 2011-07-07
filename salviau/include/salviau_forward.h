#ifndef SALVIAU_SALVIAU_FORWARD_H
#define SALVIAU_SALVIAU_FORWARD_H

#define BEGIN_NS_SALVIAU()	namespace salviau{
#define END_NS_SALVIAU()	}

#if defined(salviau_EXPORTS)
#	define SALVIAU_API __declspec(dllexport)
#else
#	define SALVIAU_API __declspec(dllimport)
#endif
#endif