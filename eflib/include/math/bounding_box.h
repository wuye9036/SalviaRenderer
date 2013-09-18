#pragma once

#include <eflib/include/platform/config.h>
#include <eflib/include/math/vector.h>
#include <eflib/include/math/matrix.h>
#include <eflib/include/math/math.h>

namespace eflib
{
	enum class intersection
	{
		unknown,
		disjoint,
		touch,
		inside,
		include,
		overlap,
		intersect
	};
	
	class bounding_box
	{
	public:
		static bounding_box create(vec3 const* verts, size_t vert_count)
		{
			bounding_box ret;
			for(size_t i = 0; i < vert_count; ++i)
			{
				ret.merge(verts+i);
			}
			return ret;
		}
		
		static bounding_box create(vec4 const* verts, size_t vert_count)
		{
			bounding_box ret;
			for(size_t i = 0; i < vert_count; ++i)
			{
				ret.merge(verts+i);
			}
			return ret;
		}
		
		bounding_box()
		{
			min_.set_ps( std::numeric_limits<float>::max() );
			max_.set_ps( std::numeric_limits<float>::min() );
		}
		
		bounding_box& merge(vec3 const* v)
		{
			min_.xyz( min_ps(reinterpret_cast<vec3&>(min_), *v) );
			max_.xyz( max_ps(reinterpret_cast<vec3&>(max_), *v) );
			return *this;
		}
		
		bounding_box& merge(vec4 const* v)
		{
			min_ = min_ps(min_, *v);
			max_ = max_ps(max_, *v);
			return *this;
		}
		
		bounding_box transform(mat44 const& mat)
		{
			if(!valid())
			{
				return *this;
			}
			
			vec4 transformed_min;
			eflib::transform(transformed_min, min_, mat);
			vec4 transformed_max;
			eflib::transform(transformed_max, max_, mat);
			
			bounding_box ret;
			ret.min_ = min_ps(transformed_min, transformed_max);
			ret.max_ = max_ps(transformed_min, transformed_max);
		}
		
		bounding_box operator & (bounding_box const& rhs)
		{
			return *this;
		}
		
		bounding_box operator + (bounding_box const& rhs)
		{
			if(!rhs.valid() || &rhs == this)
			{
				return *this;
			}
			bounding_box ret;
			ret.min_ = min_ps(min_, rhs.min_);
			ret.max_ = max_ps(max_, rhs.max_);
			
			return ret;
		}
		
		vec4 center() const
		{
			return (min_ + max_) * 0.5f;
		}
		
		vec4 radius() const
		{
			return (max_ - min_) * 0.5f;
		}
		
		bool valid() const
		{
			return min_.x() <= max_.x() && min_.y() <= max_.y() && min_.z() <= max_.z();
		}
		
		intersection intersect_test(vec3 const* pt)
		{
			if(!valid())
			{
				return intersection::unknown;
			}
			
			vec3 dist = *pt - center().xyz();
			vec3 abs_dist = vec3( abs(dist[0]), abs(dist[1]), abs(dist[2]) );
			vec3 rad = radius().xyz();
			
			if(abs_dist[0] > rad[0] || abs_dist[1] > rad[1] || abs_dist[2] > rad[2])
			{
				return intersection::disjoint;
			}
			
			if(abs_dist[0] == rad[0] || abs_dist[1] == rad[1] || abs_dist[2] == rad[2])
			{
				return intersection::touch;
			}
			
			return intersection::intersect;
		}
		
		intersection intersect_test(vec4 const* pt)
		{
			return intersect_test( reinterpret_cast<vec3 const*>(pt) );
		}
	private:
		vec4 min_;
		vec4 max_;
	};
}