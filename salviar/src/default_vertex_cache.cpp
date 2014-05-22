#include <eflib/include/platform/config.h>

#include <salviar/include/vertex_cache.h>
#include <salviar/include/stream_assembler.h>
#include <salviar/include/host.h>
#include <salviar/include/shader.h>
#include <salviar/include/shader_regs.h>
#include <salviar/include/shader_regs_op.h>
#include <salviar/include/sync_renderer.h>
#include <salviar/include/render_state.h>
#include <salviar/include/stream_assembler.h>
#include <salviar/include/thread_context.h>
#include <salviar/include/async_object.h>
#include <salviar/include/shader_unit.h>

#include <eflib/include/platform/cpuinfo.h>
#include <eflib/include/memory/pool.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/ref.hpp>
#include <eflib/include/platform/boost_end.h>

#include <atomic>
#include <memory>

#if defined(EFLIB_MSVC)
#	include <ppl.h>
#endif

using eflib::num_available_threads;

using boost::atomic;
using boost::shared_array;

using std::vector;

BEGIN_NS_SALVIAR();

const int GENERATE_INDICES_PACKAGE_SIZE = 8;
const int TRANSFORM_VERTEX_PACKAGE_SIZE = 8;

size_t const invalid_id = 0xffffffff;

#define USE_INDEX_RANGE 0

class vertex_cache_impl: public vertex_cache
{
public:
	vertex_cache_impl()
		: assembler_(nullptr), host_(nullptr)
		, prim_count_(0), prim_size_(0)
		, cpp_vs_(nullptr)
		, pipeline_stat_(nullptr), pipeline_prof_(nullptr)
		, fetch_time_stamp_(nullptr)
		, acc_ia_vertices_(nullptr), acc_vs_invocations_(nullptr)
		, acc_gather_vtx_(nullptr), acc_vtx_proc_(nullptr)
		, thread_count_( num_available_threads() )
	{
	}

	void initialize(render_stages const* stages)
	{
		assembler_	= stages->assembler.get();
		host_		= stages->host.get();
	}

	void update(render_state const* state)
	{
		// transformed_verts_.reset();
		index_fetcher_.update(state);
		cpp_vs_		= state->cpp_vs.get();
        prim_count_ = state->prim_count;

		prim_size_ = 0;
		switch(state->prim_topo)
		{
		case primitive_line_list:
		case primitive_line_strip:
			prim_size_ = 2;
			break;

		case primitive_triangle_list:
		case primitive_triangle_strip:
			prim_size_ = 3;
			break;
		}

        pipeline_stat_ = state->asyncs[static_cast<uint32_t>(async_object_ids::pipeline_statistics)].get();
		pipeline_prof_ = state->asyncs[static_cast<uint32_t>(async_object_ids::pipeline_profiles)].get();

        if(pipeline_stat_)
        {
            acc_ia_vertices_ = &async_pipeline_statistics::accumulate<pipeline_statistic_id::ia_vertices>;
            acc_vs_invocations_ = &async_pipeline_statistics::accumulate<pipeline_statistic_id::vs_invocations>;
        }
        else
        {
            acc_ia_vertices_ = &accumulate_fn<uint64_t>::null;
            acc_vs_invocations_ = &accumulate_fn<uint64_t>::null;
        }

		if(pipeline_prof_)
		{
			fetch_time_stamp_	= &async_pipeline_profiles::time_stamp;
			acc_gather_vtx_		= &async_pipeline_profiles::accumulate<pipeline_profile_id::gather_vtx>; 
			acc_vtx_proc_		= &async_pipeline_profiles::accumulate<pipeline_profile_id::vtx_proc>;
		}
		else
		{
			fetch_time_stamp_	= &time_stamp_fn::null;
			acc_gather_vtx_		= &accumulate_fn<uint64_t>::null;
			acc_vtx_proc_		= &accumulate_fn<uint64_t>::null;
		}
	}

protected:
	stream_assembler*		assembler_;
	host*					host_;

    uint32_t                prim_count_;
	uint32_t				prim_size_;
	cpp_vertex_shader*		cpp_vs_;

	index_fetcher			index_fetcher_;

    async_object*           pipeline_stat_;
	async_object*			pipeline_prof_;

	time_stamp_fn::type		fetch_time_stamp_;

    accumulate_fn<uint64_t>::type
                            acc_ia_vertices_;
    accumulate_fn<uint64_t>::type
                            acc_vs_invocations_;
	accumulate_fn<uint64_t>::type
							acc_gather_vtx_;
	accumulate_fn<uint64_t>::type
							acc_vtx_proc_;

	size_t					thread_count_;
};

class default_vertex_cache : public vertex_cache_impl
{
public:
	default_vertex_cache()
		: transformed_verts_capacity_ (0)
	{
	}

	void prepare_vertices()
	{
		uint64_t gather_vtx_start_time = fetch_time_stamp_();

		indices_.resize(prim_count_ * prim_size_);

		// Generate indices
		min_index_ = std::numeric_limits<uint32_t>::max();
		max_index_ = 0;

		auto generate_indicies_ = [this](thread_context* ctx)
		{
			this->generate_indices(ctx);
		};
		execute_threads(generate_indicies_, prim_count_, GENERATE_INDICES_PACKAGE_SIZE);

		uint32_t verts_count = 0;

#if !USE_INDEX_RANGE
		// Unique indices
		unique_indices_ = indices_;
#if defined(EFLIB_MSVC)
		concurrency::parallel_radixsort(unique_indices_.begin(), unique_indices_.end());
#else
		std::sort(unique_indices_.begin(), unique_indices_.end());
#endif
		unique_indices_.erase(std::unique(unique_indices_.begin(), unique_indices_.end()), unique_indices_.end());

		verts_count = static_cast<uint32_t>( unique_indices_.size() );
		if( transformed_verts_capacity_ < unique_indices_.size() )
		{
			transformed_verts_.reset(new vs_output[unique_indices_.size()]);
			transformed_verts_capacity_ = unique_indices_.size();
		}
		
		used_verts_.resize( unique_indices_.back()+1 );
#else
		uint32_t verts_count = max_index - min_index_ + 1;
		if(transformed_verts_capacity_ < verts_count)
		{
			transformed_verts_.reset(new vs_output[verts_count]);
			transformed_verts_capacity_ = verts_count;
		}
#endif

        // Accumulate query counters.
        acc_ia_vertices_( pipeline_stat_, static_cast<uint64_t>(prim_count_*prim_size_) );
        acc_vs_invocations_( pipeline_stat_, static_cast<uint64_t>(verts_count) );
		acc_gather_vtx_(pipeline_prof_, fetch_time_stamp_() - gather_vtx_start_time);

		// Transform vertexes
		if( cpp_vs_ )
		{
			assembler_->update_register_map( cpp_vs_->get_register_map() );
		}

		uint64_t vtx_proc_start_time = fetch_time_stamp_();

		if(cpp_vs_)
		{
			auto execute_vert_shader = [this](thread_context* thread_ctx)
			{
				this->transform_vertex_cppvs(thread_ctx);
			};
			execute_threads(execute_vert_shader, verts_count, TRANSFORM_VERTEX_PACKAGE_SIZE);
		}
		else
		{
			auto execute_vert_shader = [this](thread_context* thread_ctx)
			{
				this->transform_vertex_vs(thread_ctx);
			};
			execute_threads(execute_vert_shader, verts_count, TRANSFORM_VERTEX_PACKAGE_SIZE);
		}

		acc_vtx_proc_(pipeline_prof_, fetch_time_stamp_() - vtx_proc_start_time);
	}

	void fetch3(vs_output** v, cache_entry_index id, uint32_t /*thread_id*/)
	{
		static vs_output null_obj;
		uint32_t id0 = indices_[id];
		uint32_t id1 = indices_[id+1];
		uint32_t id2 = indices_[id+2];
#if !USE_INDEX_RANGE
		
#if defined(EFLIB_DEBUG)
		if((id2 > used_verts_.size()) || (-1 == used_verts_[id0]) || (-1 == used_verts_[id1]) || (-1 == used_verts_[id2]))
		{
			assert( !"The vertex could not be transformed. Maybe errors occurred on index statistics or vertex tranformation." );
			// return null_obj;
		}
#endif

		v[0] = &transformed_verts_[used_verts_[id0]];
		v[1] = &transformed_verts_[used_verts_[id1]];
		v[2] = &transformed_verts_[used_verts_[id2]];
#else
		return transformed_verts_[id - min_index_];
#endif
	}

private:
	void generate_indices(thread_context const* thread_ctx)
	{
		// Fetch indexes and min/max of package
		uint32_t thread_min_index = std::numeric_limits<uint32_t>::max();
		uint32_t thread_max_index = 0;

		thread_context::package_cursor current_package = thread_ctx->next_package();
		while ( current_package.valid() )
		{
			auto prim_range = current_package.item_range();
			uint32_t pkg_min_index, pkg_max_index;
			index_fetcher_.fetch_indexes(&indices_[prim_range.first*prim_size_], &pkg_min_index, &pkg_max_index, prim_range.first, prim_range.second);
			thread_min_index = std::min(pkg_min_index, thread_min_index);
			thread_max_index = std::max(pkg_max_index, thread_max_index);
			current_package = thread_ctx->next_package();
		}

		// While thread is ended, merge min/max index to global.
		uint32_t old_min_index = 0;
		uint32_t new_min_index = 0;
		do
		{
			old_min_index = min_index_;	
			new_min_index = std::min(old_min_index, thread_min_index);
		}
		while( !min_index_.compare_exchange_weak(old_min_index, new_min_index) );

		uint32_t old_max_index = 0;
		uint32_t new_max_index = 0;
		do
		{
			old_max_index = max_index_;
			new_max_index = std::max(old_max_index, thread_max_index);
		}
		while( !max_index_.compare_exchange_weak(old_max_index, new_max_index) );
	}

#if !USE_INDEX_RANGE
	void transform_vertex_cppvs(thread_context* thread_ctx)
	{
		thread_context::package_cursor current_package = thread_ctx->next_package();
		while ( current_package.valid() )
		{
			auto vert_range = current_package.item_range();
			for(auto i = vert_range.first; i < vert_range.second; ++i)
			{
				uint32_t id = unique_indices_[i];
				used_verts_[id] = i;
				vs_input vertex;
				assembler_->fetch_vertex(vertex, id);
				cpp_vs_->execute(vertex, transformed_verts_[i]);
			}
			current_package = thread_ctx->next_package();
		}
	}

	void transform_vertex_vs(thread_context* thread_ctx)
	{
		vx_shader_unit_ptr vsu	= host_->get_vx_shader_unit();

		thread_context::package_cursor current_package = thread_ctx->next_package();
		while ( current_package.valid() )
		{
			auto vert_range = current_package.item_range();
			for(auto i = vert_range.first; i < vert_range.second; ++i)
			{
				uint32_t vert_index = unique_indices_[i];
				used_verts_[vert_index] = i;
				vsu->execute(vert_index, transformed_verts_[i]);
			}
			current_package = thread_ctx->next_package();
		}
	}
#else
	void transform_vertex_cppvs(thread_context* thread_ctx)
	{
		thread_context::package_cursor current_package = thread_ctx->next_package();
		while ( current_package.valid() )
		{
			auto vert_range = current_package.item_range();
			for(auto i = vert_range.first; i < vert_range.second; ++i)
			{
				vs_input vertex;
				assembler_->fetch_vertex(vertex, i + min_index_);
				cpp_vs_->execute(vertex, transformed_verts_[i]);
			}
			current_package = thread_ctx->next_package();
		}
	}

	void transform_vertex_vs(thread_context* thread_ctx)
	{
		vx_shader_unit_ptr vsu = host_->get_vx_shader_unit();
		thread_context::package_cursor current_package = thread_ctx->next_package();
		while ( current_package.valid() )
		{
			auto vert_range = current_package.item_range();
			for(auto i = vert_range.first; i < vert_range.second; ++i)
			{
				vsu->execute(i + min_index_, transformed_verts_[i]);
			}
			current_package = thread_ctx->next_package();
		}
	}
#endif

private:
	vector<uint32_t>		indices_;
	vector<uint32_t>		unique_indices_;

	shared_array<vs_output> transformed_verts_;
	size_t					transformed_verts_capacity_;

	vector<int32_t>			used_verts_;

	std::atomic<uint32_t>	min_index_;
	std::atomic<uint32_t>	max_index_;
};

class tls_vertex_cache: public vertex_cache_impl
{
	tls_vertex_cache()
	{
	}
	
	void fetch3(vs_output** v, uint32_t prim, uint32_t thread_id)
	{
		uint32_t indexes[3];
		uint32_t min_index, max_index;
		index_fetcher_.fetch_indexes(indexes, &min_index, &max_index, prim, prim+1);

		auto& cache = caches_[thread_id];

		for(int i = 0; i < 3; ++i)
		{
			uint32_t index = indexes[i];
			uint32_t key = indexes[i] % ENTRY_SIZE;
			auto& cache_item = cache.items[key];
			if(cache_item.first == indexes[i])
			{
				v[i] = cache_item.second;
			}
			else
			{
				auto ret = cache.vso_pool.alloc();
				
				if(cpp_vs_)
				{
					vs_input vertex;
					assembler_->fetch_vertex(vertex, index);
					cpp_vs_->execute(vertex, *ret);
				}
				else
				{
					cache.vsu->execute(index, *ret);
				}

				cache_item = std::make_pair(index, ret);
				v[i] = ret;
			}
		}
	}

private:
	static const int ENTRY_SIZE = 32;
	
	struct thread_cache
	{
		std::pair<uint32_t, vs_output*>			items[ENTRY_SIZE];
		eflib::pool::reserved_pool<vs_output>	vso_pool;
		vx_shader_unit_ptr						vsu;	
	};

	std::vector<thread_cache>	caches_;
};

vertex_cache_ptr create_default_vertex_cache()
{
	return vertex_cache_ptr( new default_vertex_cache() );
}

END_NS_SALVIAR();
