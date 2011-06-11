#ifndef EFLIB_COLLISION_DETECTION_H
#define EFLIB_COLLISION_DETECTION_H

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/math/vector.h>

namespace eflib{

	template<class T>
	struct rect
	{
		T x, y, w, h;

		rect():x(T(0)), y(T(0)), w(T(0)), h(T(0)){
		}

		rect(T x, T y, T w, T h):x(x), y(y), w(w), h(h){
			//EFLIB_ASSERT(w > 0 && h > 0, "");
		}

		template<class U>
		rect(const rect<U>& rhs):x(T(rhs.x)), y((T)(rhs.y)), w((T)(rhs.w)), h((T)(rhs.h)){}

		template<class U>
		rect<T>&  operator = (const rect<U>& rhs)
		{
			x = (T)(rhs.x);
			y = (T)(rhs.y);
			w = (T)(rhs.w);
			h = (T)(rhs.h);
			return *this;
		}

		void set_min(eflib::vec4& min)
		{
			x = (T)(min.x);
			y = (T)(min.y);
		}

		void set_max(eflib::vec4& max)
		{
			EFLIB_ASSERT(max.x > x && max.y > y, "");
			w = (T)(max.x) - x;
			h = (T)(max.y) - y;
		}

		eflib::vec4 get_min()
		{
			return eflib::vec4((float)x, (float)y, 0.0f, 0.0f);
		}

		eflib::vec4 get_max()
		{
			return eflib::vec4(float(x + w), float(y + h), 0.0f, 0.0f);
		}

		template<class U>
		bool is_overlapped(const rect<U>& rc)
		{
			if(U(rc.x) > x + w) return false;
			if(U(rc.y) > y + h) return false;
			if(U(rc.x + rc.w) < x) return false;
			if(U(rc.y + rc.h) < y) return false;
			return true;
		}
	};

	template <int demension>
	class AABB
	{
	public:
		vec4 min_vert;
		vec4 max_vert;

		AABB()
		{
		}

		AABB(const vec4* pverts, int n)
		{
			set_boundary(pverts, n);
		}

		void set_boundary(const vec4* pverts, int n)
		{
			max_vert = min_vert = pverts[0];
			append_vertex(pverts + 1, n - 1);
		}

		void append_vertex(const vec4* pverts, int n)
		{
			for(int ivert = 0; ivert < n; ++ivert)
			{
				for(int ielem = 0; ielem < demension; ++ielem)
				{
					if(pverts[ivert][ielem] < min_vert[ielem])
					{
						min_vert[ielem] = pverts[ivert][ielem];
					}
					else
					{					
						if(pverts[ivert][ielem] > max_vert[ielem])
						{
							max_vert[ielem] = pverts[ivert][ielem];
						}
					}
				}
			}
		}

		bool is_intersect(const AABB<demension>& rhs) const
		{
			for(int i = 0; i < demension; ++i)
			{
				if(min_vert[i] > rhs.max_vert[i]) return false;
				if(max_vert[i] < rhs.min_vert[i]) return false;
			}
			return true;
		}

		vec4 get_center() const
		{
			return (min_vert + max_vert) / 2.0f;
		}

		vec4 get_half_size()const
		{
			return (max_vert - min_vert) / 2.0f;
		}

		void get_center_size(vec4& c, vec4& hs) const
		{
			c = get_center();
			hs = max_vert - c;
		}
	};

	typedef AABB<3> AABB_3D;

	bool is_tri_cube_overlap(const AABB_3D& box, const vec4& v0, const vec4& v1, const vec4& v2);
}
#endif