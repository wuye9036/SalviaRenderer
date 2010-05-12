#ifndef SOFTART_CPUINFO_H
#define SOFTART_CPUINFO_H
#include "softart_fwd.h"
BEGIN_NS_SOFTART()

uint32_t num_cpu_cores();
uint32_t num_available_threads();

END_NS_SOFTART()
#endif
