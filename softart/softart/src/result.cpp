#include "softart/include/enums.h"
#include <tchar.h>
const result result::ok(0, _T("no error occurs."));
const result result::failed(1, _T("process failed with unknown reason."));
const result result::outofmemory(2, _T("process failed for out of memory!"));
const result result::invalid_parameter(3, _T("process failed for invalid parameter!"));