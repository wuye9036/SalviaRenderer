#pragma once

#include <utility>

#define EF_FORWARD(e) std::forward<decltype(e)>(e)

#define EF_THIS_MEM_FN(fn_name) std::remove_pointer_t<decltype(this)>::fn_name