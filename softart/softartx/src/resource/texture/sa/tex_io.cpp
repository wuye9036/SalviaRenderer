/*
Copyright (C) 2007-2010 Minmin Gong, Ye Wu

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
*/

#include "softartx/include/resource/texture/sa/tex_io.h"
#include "softart/include/surface.h"
#include "eflib/include/unicode_utility.h"

#include <fstream>
#include <cstdio>
#include <tchar.h>

using namespace std;
using namespace efl;
using namespace softart;
BEGIN_NS_SOFTARTX_RESOURCE()

//Impl Utilities
struct raw_header
{
	size_t width;
	size_t height;
	size_t format;
};

END_NS_SOFTARTX_RESOURCE()