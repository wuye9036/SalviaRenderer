/*
Copyright (C) 2004-2005 Minmin Gong

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

#include "softartx/include/resource/tex_io.h"

#include "softart/include/surface.h"

#include "eflib/include/unicode_utility.h"

#include <tchar.h>

#include <fstream>

#include <cstdio>

using namespace std;
using namespace efl;

struct raw_header
{
	size_t width;
	size_t height;
	size_t format;
};

void save_surface_to_raw(surface& surf, const _tstring& filename, pixel_format format )
{
	FILE* outf = _tfopen(filename.c_str(), _T("wb"));
	custom_assert(outf, "");
	if(!outf){
		return;
	}

	byte* pdata = NULL;
	void* pbuf = new char[surf.get_width()*color_infos[format].size];
	surf.lock((void**)&pdata, rect<size_t>(0, 0, surf.get_width(), surf.get_height()), lock_read_only);

	raw_header rh;
	rh.format = size_t(format);
	rh.height = surf.get_height();
	rh.width = surf.get_width();

	//save header file
	fwrite(&rh, sizeof(raw_header), 1, outf);

	for(size_t ih = 0; ih < surf.get_height(); ++ih)
	{
		pixel_format_convertor::convert_array(format, surf.get_pixel_format(), pbuf, pdata, int(surf.get_width()));
		fwrite(pbuf, surf.get_width()*color_infos[format].size, 1, outf);
		pdata += surf.get_width()*color_infos[surf.get_pixel_format()].size;
	}
	surf.unlock();
	fclose(outf);
}

//channel = ‘RGBA’则依次输出RGBA，如果为‘RGB'则输出RGB，如果为'BGR'则输出BGR
void save_surface_to_raw_text( surface& surf, const std::_tstring& filename, uint32_t channel)
{
	const pixel_information& color_info = color_infos[pixel_format_color_rgba32f];

	_tfstream outf(to_ansi_string(filename).c_str(), ios::out);

	custom_assert(outf, "");
	if(!outf){
		return;
	}

	//save header file
	outf << to_tstring(color_info.describe) << " ";
	outf << surf.get_width() << " ";
	outf << surf.get_height() << endl;

	for(size_t ih = 0; ih < surf.get_height(); ++ih)
	{
		for(size_t iw = 0; iw < surf.get_width(); ++iw)
		{
			color_rgba32f color = surf.get_texel(iw, ih);
			for(int i_ch = 0; i_ch < 4; ++i_ch)
			{
				bool not_first_ch = false;

				switch (*((byte*)(&channel)+i_ch))
				{
				default:
					if (not_first_ch){
						outf << ",";
					}
					break;
				case 'R':
					outf << color.r;
					not_first_ch = true;
					break;
				case 'G':
					outf << color.g;
					not_first_ch = true;
					break;					
				case 'B':
					outf << color.b;
					not_first_ch = true;
					break;
				case 'A':
					outf << color.a;
					not_first_ch = true;
					break;
				}
			}
			outf << ";";
		}
		outf << endl;
	}
	
	outf.close();
}