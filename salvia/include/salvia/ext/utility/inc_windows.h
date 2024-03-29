/********************************************************************
Copyright (C) 2007-2008 Ye Wu

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

created:	2008/06/08
author:		Ye Wu

purpose:

Modify Log:

*********************************************************************/
#ifndef SALVIA_EXT_INC_WINDOWS_H
#define SALVIA_EXT_INC_WINDOWS_H

#ifndef EFLIB_USE_STD_MINMAX
#  include <algorithm>
using ::std::max;
using ::std::min;
#  define NOMINMAX
#  define EFLIB_USE_STD_MINMAX
#endif
#include <windows.h>

#endif  // SALVIA_EXT_INC_WINDOWS_H