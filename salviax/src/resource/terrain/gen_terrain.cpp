/* All Algorithms Copyright (C) Jason Shankel, 2000. */
#include <salviax/include/resource/terrain/gen_terrain.h>

#include <salviar/include/texture.h>
#include <salviar/include/renderer.h>
#include <salviar/include/mapped_resource.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/addressof.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>
#include <vector>

#include <stdlib.h>
#include <math.h>
#include <memory.h>

EFLIB_USING_SHARED_PTR(salviar, texture);
using salviar::renderer;

using std::vector;

BEGIN_NS_SALVIAX_RESOURCE();

void normalize_terrain(vector<float>& field);
void filter_terrain_band(float* band, int stride, int count, float filter);

// Returns a random number between v1 and v2
float gen_random(float lower, float upper)
{
	return lower + (upper-lower)*((float)rand())/((float)RAND_MAX);
}

/*
Given a height field, normalize it so that the minimum altitude
is 0.0 and the maximum altitude is 1.0
*/
void normalize_terrain(vector<float>& field, int size)
{
	/*
	Find the maximum and minimum values in the height field
	*/
	std::pair<vector<float>::iterator, vector<float>::iterator>
		max_min_val = std::minmax_element( field.begin(), field.end() );
	float min_val = *max_min_val.first;
	float max_val = *max_min_val.second;

	// Find the altitude range (dh)
	if (max_val <= min_val) return;
	float inv_dh = 1.0f / (max_val - min_val);

	// Scale all the values so they are in the range 0-1
	for(int i = 0; i < size*size; ++i)
	{
		field[i] = (field[i] - min_val) * inv_dh;
	}
}

// Erosion filter -
//	filter_terrain_band applies a FIR filter across a row or column of the height field
void filter_terrain_band(float *band, int stride, int count, float filter)
{
	int pos = stride;
	float v = band[0];
	for (int i_band = 0 ; i_band < count - 1; ++i_band)
	{
		band[pos] = filter * v + (1 - filter) * band[pos];
		v = band[pos];
		pos += stride;
	}
}

// Erosion filter - Erodes a terrain in all 4 directions
void filter_terrain(vector<float>& field, int size, float filter)
{
	// Erode rows left to right
	for (int i = 0; i < size; ++i)
	{
		filter_terrain_band(&field[size*i], 1, size, filter);
	}

	// Erode rows right to left
	for (int i = 0; i < size; ++i)
	{
		filter_terrain_band(&field[size*i+size-1], -1, size, filter);
	}

	// Erode columns top to bottom
	for (int i = 0; i < size; ++i)
	{
		filter_terrain_band(&field[i], size, size, filter);
	}

	// Erode columns bottom to top
	for (int i = 0; i < size; ++i)
	{
		filter_terrain_band(&field[size*(size-1)+i], -size, size, filter);
	}
}

// Generate terrain using diamond-square (plasma) algorithm
void make_terrain_plasma(vector<float>& field, int _size, float rough)
{
	field.resize(_size*_size);

	int size = _size;
	int rect_size = size;

	float dh = rect_size / 2.0f;
	float r  = powf(2.0f, -rough);

	/*
	Since the terrain wraps, all 4 "corners" are represented by the value at 0,0,
	so seeding the height field is very straightforward
	Note that it doesn't matter what we use for a seed value, since we're going to
	re-normalize the terrain after we're done
	*/
	field[0] = 1.0f;

	while(rect_size > 0)
	{
		// Rectangle Center
		/*
		a   b
		c
		d   e
		*/
		for (int i = 0; i < size; i+=rect_size)
		{
			for (int j = 0; j < size; j+=rect_size)
			{
				int ni = (i + rect_size) % size;
				int nj = (j + rect_size) % size;

				int mi = (i + rect_size/2) % size;
				int mj = (j + rect_size/2) % size;

				float neibourgh_total_height = 
					field[i+j*_size] + field[ni+j*_size] + field[i+nj*_size] + field[ni+nj*_size];
				field[mi+mj*_size] = neibourgh_total_height / 4 + gen_random(-dh/2, dh/2);
			}
		}

		// Diamond Center
		for (int i = 0; i < size; i+=rect_size)
		{
			for (int j=0; j < size; j+=rect_size)
			{
				int ni = (i + rect_size) % size;
				int nj = (j + rect_size) % size;

				int mi = (i + rect_size/2) % size;
				int mj = (j + rect_size/2) % size;

				int pmi = (i - rect_size/2 + size) % size;
				int pmj = (j - rect_size/2 + size) % size;

				field[mi+j*_size] =
					(field[i+j*_size] + field[ni+j*_size] + field[mi+pmj*_size] + field[mi+mj*_size]) / 4
					+ gen_random(-dh/2, dh/2);

				field[i+mj*_size] =
					(field[i+j*_size] + field[i+nj*_size] + field[pmi+mj*_size] + field[mi+mj*_size]) / 4
					+ gen_random(-dh/2, dh/2);

			}
		}

		/*
		Setup values for next iteration
		At this point, the height field has valid values at each of the coordinates that fall on a rect_size/2 boundary
		*/
		rect_size  = rect_size >> 1;
		dh *= r;
	}

	// Normalize terrain so minimum value is 0 and maximum value is 1
	normalize_terrain(field, size);
}

void make_terrain_fault(
	vector<float>& field, int size,
	int iterations, int max_delta, int min_delta, int iterations_per_filter, float filter
	)
{
	field.resize(size*size);

	// Clear the height field
	for (int i = 0; i < size*size; ++i)
	{
		field[i] = 0.0f;
	}

	for (int i = 0; i < iterations; ++i)
	{
		/*
		Calculate the dh for this iteration
		(linear interpolation from max_delta to min_delta)
		*/
		int dh = max_delta - ( (max_delta - min_delta) * i ) / iterations;

		/*
		Pick two random points on the field for the line
		(make sure they're not identical)
		*/
		int x1 = rand() % size;
		int y1 = rand() % size;

		int x2 = 0;
		int y2 = 0;
		do {
			x2 = rand() % size;
			y2 = rand() % size;
		} while ( x2 == x1 && y2 == y1);

		// dx1,dy1 is a vector in the direction of the line
		int dx1 = x2 - x1;
		int dy1 = y2 - y1;

		for (int x2 = 0; x2 < size; x2++)
		{
			for (int y2 = 0; y2 < size; y2++)
			{
				// dx2,dy2 is a vector from x1,y1 to the candidate point
				int dx2 = x2-x1;
				int dy2 = y2-y1;

				// if z component of the cross product is 'up', then elevate this point
				if (dx2*dy1 - dx1*dy2 > 0)
				{
					field[x2+size*y2] += (float)(dh);
				}
			}
		}

		// Erode terrain
		if (iterations_per_filter != 0 && (i % iterations_per_filter) == 0)
		{
			filter_terrain(field, size, filter);
		}
	}

	// Normalize terrain (height field values in the range 0-1)
	normalize_terrain(field, size);
}

texture_ptr make_terrain_texture(renderer* rend, std::vector<float>& normalized_field, int size )
{
	texture_ptr ret = rend->create_tex2d(size, size, 1, salviar::pixel_format_color_r32f);
	
	salviar::mapped_resource mapped;

	rend->map(mapped, ret->subresource(0), salviar::map_write);
	for(size_t y = 0; y < size; ++y)
	{
		uint8_t* dst_line = reinterpret_cast<uint8_t*>(mapped.data) + y * mapped.row_pitch;
		float*	 src_line = normalized_field.data() + y * size;
		memcpy(dst_line, src_line, size * sizeof(float) );
	}
	rend->unmap();
	
	return ret;
}

END_NS_SALVIAX_RESOURCE();
