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
#include <eflib/include/memory/allocator.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/ref.hpp>
#include <eflib/include/platform/boost_end.h>

#include <atomic>
#include <memory>
#include <iostream>

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

class precomputed_vertex_cache : public vertex_cache_impl
{
public:
	precomputed_vertex_cache()
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

	void fetch3(vs_output** v, cache_entry_index prim, uint32_t /*thread_id*/)
	{
		static vs_output null_obj;
		uint32_t id0 = indices_[prim*3+0];
		uint32_t id1 = indices_[prim*3+1];
		uint32_t id2 = indices_[prim*3+2];
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

	void update_statistic()
	{
		// do nothing
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
public:
	tls_vertex_cache(): caches_( num_available_threads() )
	{
	}

	void prepare_vertices()
	{
		for(uint32_t i = 0; i < num_available_threads(); ++i)
		{
			auto& cache = caches_[i];
			cache.vso_pool.clear();
			cache.vso_pool.reserve(prim_count_ * prim_size_, 16);
			for(int j = 0; j < ENTRY_SIZE; ++j)
			{
				cache.items[j] = std::make_pair(std::numeric_limits<uint32_t>::max(), nullptr);
			}
			if(host_) cache.vsu = host_->get_vx_shader_unit();
			cache.ia_vertices = 0;
			cache.vs_invocations = 0;
			cache.vs_during = 0;
		}
	}
	
	void fetch3(vs_output** v, cache_entry_index prim, uint32_t thread_id)
	{
		// uint64_t vs_start_time = fetch_time_stamp_();

		uint32_t indexes[3];
		uint32_t min_index, max_index;
		index_fetcher_.fetch_indexes(indexes, &min_index, &max_index, prim, prim+1);

		auto& cache = caches_[thread_id];

		cache.ia_vertices += 3;

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
				++cache.vs_invocations;

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

		// cache.vs_during += fetch_time_stamp_() - vs_start_time;
	}

	void update_statistic() override
	{
		for(uint32_t i = 0; i < num_available_threads(); ++i)
		{
			auto& cache = caches_[i];
			
			acc_ia_vertices_(pipeline_stat_, cache.ia_vertices);
			cache.ia_vertices = 0;

			acc_vs_invocations_(pipeline_stat_, cache.vs_invocations);
			cache.vs_invocations = 0;

			acc_vtx_proc_(pipeline_prof_, cache.vs_during);
			cache.vs_during = 0;
		}
	}
private:
	static const int ENTRY_SIZE = 128;
	
	struct EFLIB_ALIGN(64) thread_cache
	{
		thread_cache(){}
		thread_cache(thread_cache const&) {}
		thread_cache& operator = (thread_cache const&) { return *this; }

		eflib::pool::reserved_pool<vs_output>	vso_pool;
		vx_shader_unit_ptr						vsu;
		uint64_t								vs_invocations;
		uint64_t								ia_vertices;
		uint64_t								vs_during;

		std::pair<uint32_t, vs_output*>			items[ENTRY_SIZE];
	};

	std::vector<
		thread_cache,
		eflib::aligned_allocator<thread_cache, 64>
	>	caches_;
};

class shared_vertex_cache: public vertex_cache_impl
{
public:
	shared_vertex_cache(): caches_( num_available_threads() )
	{
		conflict_count_ = 0;
		l2_missing_ = 0;
		l2_hitting_ = 0;
	}
	~shared_vertex_cache()
	{
		std::cout << "Conflict: " << conflict_count_ << std::endl;
		std::cout << "L2 Missing: " << l2_missing_ << std::endl;
		std::cout << "L2 Hitting: " << l2_hitting_ << std::endl;
	}
	void prepare_vertices()
	{
		memset(shared_items_, INVALID_SHARED_ENTRY, sizeof(shared_items_));

		for(uint32_t i = 0; i < num_available_threads(); ++i)
		{
			auto& cache = caches_[i];
			cache.vso_pool.clear();
			cache.vso_pool.reserve(prim_count_ * prim_size_, 16);
			for(int j = 0; j < ENTRY_SIZE; ++j)
			{
				cache.items[j] = std::make_pair(std::numeric_limits<uint32_t>::max(), nullptr);
			}
			if(host_) cache.vsu = host_->get_vx_shader_unit();
			cache.ia_vertices = 0;
			cache.vs_invocations = 0;
			cache.vs_during = 0;
			cache.conflict_count = 0;
			cache.l2_missing = 0;
			cache.l2_hitting = 0;
		}
	}
	
	inline uint32_t lock_shared_item(uint32_t& conflict_count, std::pair<std::atomic<uint32_t>, vs_output*>& item)
	{
		uint32_t index_in_cache;
				
		for(;;)
		{	
			index_in_cache = item.first.load(std::memory_order_acquire);
			if(index_in_cache == SHARED_ENTRY_IS_USING)
			{
				++conflict_count;
				continue;
			}

			if( item.first.compare_exchange_weak(index_in_cache, SHARED_ENTRY_IS_USING) )
			{
				return index_in_cache;
			}
			++conflict_count;
		}
	}
	
	inline void release_shared_item(std::pair<std::atomic<uint32_t>, vs_output*>& item, uint32_t index)
	{
		item.first = index;
	}


	void fetch3(vs_output** v, cache_entry_index prim, uint32_t thread_id)
	{
		// uint64_t vs_start_time = fetch_time_stamp_();

		uint32_t indexes[3];
		uint32_t min_index, max_index;
		index_fetcher_.fetch_indexes(indexes, &min_index, &max_index, prim, prim+1);

		auto& cache = caches_[thread_id];

		cache.ia_vertices += 3;

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
				uint32_t sc_key = index % SHARED_ENTRY_SIZE;
				auto& shared_item = shared_items_[sc_key];
				uint32_t index_in_cache = lock_shared_item(cache.conflict_count, shared_item);

				if(index_in_cache == index)
				{
					cache_item = std::make_pair(index, shared_item.second);
					release_shared_item(shared_item, index);

					v[i] = cache_item.second;
					++cache.l2_hitting;
				}
				else
				{
					release_shared_item(shared_item, index_in_cache);

					if(index_in_cache != INVALID_SHARED_ENTRY)
					{
						++cache.l2_missing;
					}
					++cache.vs_invocations;

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

					lock_shared_item(cache.conflict_count, shared_item);
					shared_items_[sc_key].second = ret;
					release_shared_item(shared_item, index);
				}
			}
		}

		// cache.vs_during += fetch_time_stamp_() - vs_start_time;
	}

	void update_statistic() override
	{
		for(uint32_t i = 0; i < num_available_threads(); ++i)
		{
			auto& cache = caches_[i];
			
			acc_ia_vertices_(pipeline_stat_, cache.ia_vertices);
			cache.ia_vertices = 0;

			acc_vs_invocations_(pipeline_stat_, cache.vs_invocations);
			cache.vs_invocations = 0;

			acc_vtx_proc_(pipeline_prof_, cache.vs_during);
			cache.vs_during = 0;

			conflict_count_ += cache.conflict_count;
			l2_missing_ += cache.l2_missing;
			l2_hitting_ += cache.l2_hitting;
		}
	}
private:
	static int const		SHARED_ENTRY_SIZE = 1024;
	static int const		ENTRY_SIZE = 32;
	static uint32_t const	SHARED_ENTRY_IS_USING = 0xFFFFFFFEU;
	static uint32_t const	INVALID_SHARED_ENTRY  = 0xFFFFFFFFU;
	
	struct EFLIB_ALIGN(64) thread_cache
	{
		thread_cache(){}
		thread_cache(thread_cache const&) {}
		thread_cache& operator = (thread_cache const&) { return *this; }

		uint32_t								conflict_count;
		uint32_t								l2_hitting;
		uint32_t								l2_missing;

		eflib::pool::reserved_pool<vs_output>	vso_pool;
		vx_shader_unit_ptr						vsu;
		uint64_t								vs_invocations;
		uint64_t								ia_vertices;
		uint64_t								vs_during;

		std::pair<uint32_t, vs_output*>			items[ENTRY_SIZE];
	};

	uint32_t				conflict_count_;
	uint32_t				l2_missing_;
	uint32_t				l2_hitting_;
	
	std::vector<
		thread_cache,
		eflib::aligned_allocator<thread_cache, 64>
	>						caches_;
	std::pair<std::atomic<uint32_t>, vs_output*>
							shared_items_[SHARED_ENTRY_SIZE];
};

vertex_cache_ptr create_default_vertex_cache()
{
	return vertex_cache_ptr( new tls_vertex_cache() );
}

END_NS_SALVIAR();
