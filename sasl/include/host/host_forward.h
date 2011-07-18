#ifndef SASL_HOST_HOST_FORWARD_H
#define SASL_HOST_HOST_FORWARD_H

#define BEGIN_NS_SASL_HOST() namespace sasl{ namespace host{
#define END_NS_SASL_HOST() }}

#ifdef sasl_host_EXPORTS
#define SASL_HOST_API __declspec(dllexport)
#else
#define SASL_HOST_API __declspec(dllimport)
#endif

#endif