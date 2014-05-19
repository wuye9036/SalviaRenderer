#pragma once

#include <salviar/include/salviar_forward.h>

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/atomic.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/array.hpp>
#include <boost/chrono.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SALVIAR();

class async_object
{
protected:
    int32_t		    	    pending_writes_count_; 
    bool                    started_;               // Mark to prevent invalid call such as BEG-GET-END, END-BEG, BEG-BEG cases.
	
	boost::mutex		    pending_writes_mutex_;
	boost::condition	    pending_writes_condition_;

public:
    async_object(): started_(false), pending_writes_count_(0)
    {
    }

    virtual async_object_ids id()
    {
        return async_object_ids::none;
    }

    uint32_t uint_id()
    {
        return static_cast<uint32_t>(id());
    }

    bool begin()
    {
        if(started_)
        {
            return false;
        }
        ++pending_writes_count_;
        started_ = true;

        return true;
    }
	
    bool end()
    {
        if(!started_)
        {
            return false;
        }
        started_ = false;

        return true;
    }
	
	void start_counting()
    {
        init_async_data();
    }

	void stop_counting()
	{
        boost::lock_guard<boost::mutex> lock(pending_writes_mutex_);
		
        if(--pending_writes_count_ == 0)
		{
			pending_writes_condition_.notify_one();
		}
	}

    async_status get(void* ret, bool do_not_wait)
    {
        if(started_)
        {
            return async_status::error;
        }
		
		{	
            boost::lock_guard<boost::mutex> lock(pending_writes_mutex_);
			
			while(pending_writes_count_ > 0)
			{
				if(do_not_wait)
				{
					return async_status::timeout;
				}
				
				pending_writes_condition_.wait(pending_writes_mutex_);
			}
		}
		
		get_value(ret);
        return async_status::ready;
    }

    virtual ~async_object(){}

protected:
    virtual void get_value(void*) = 0;
    virtual void init_async_data() = 0;
};

struct pipeline_statistics
{
    uint64_t ia_vertices;
    uint64_t ia_primitives;
    uint64_t vs_invocations;
    uint64_t gs_invocations;
    uint64_t gs_primitives;
    uint64_t cinvocations;
    uint64_t cprimitives;
    uint64_t ps_invocations;
};

enum class pipeline_statistic_id: uint32_t
{
    ia_vertices = 0,
    ia_primitives,
    vs_invocations,
    gs_invocations,
    gs_primitives,
    cinvocations,
    cprimitives,
    ps_invocations
};

class async_pipeline_statistics: public async_object
{
public:
    template <pipeline_statistic_id StatID>
    static void accumulate(async_object* query_obj, uint64_t v)
    {
        assert(dynamic_cast<async_pipeline_statistics*>(query_obj) != nullptr);
	    static_cast<async_pipeline_statistics*>(query_obj)->counters_[static_cast<uint32_t>(StatID)] += v;
    }

    virtual async_object_ids id()
    {
        return async_object_ids::pipeline_statistics;
    }

protected:
    boost::array<boost::atomic<uint64_t>, 8> counters_;

    void get_value(void* v)
    {
        auto ret = reinterpret_cast<pipeline_statistics*>(v);
        ret->cinvocations = counters_[static_cast<uint32_t>(pipeline_statistic_id::cinvocations)];
        ret->cprimitives = counters_[static_cast<uint32_t>(pipeline_statistic_id::cprimitives)];
        ret->gs_invocations = counters_[static_cast<uint32_t>(pipeline_statistic_id::gs_invocations)];
        ret->gs_primitives = counters_[static_cast<uint32_t>(pipeline_statistic_id::gs_primitives)];
        ret->ia_primitives = counters_[static_cast<uint32_t>(pipeline_statistic_id::ia_primitives)];
        ret->ia_vertices = counters_[static_cast<uint32_t>(pipeline_statistic_id::ia_vertices)];
        ret->ps_invocations = counters_[static_cast<uint32_t>(pipeline_statistic_id::ps_invocations)];
        ret->vs_invocations = counters_[static_cast<uint32_t>(pipeline_statistic_id::vs_invocations)];
    }

    virtual void init_async_data()
    {
        for(auto& counter: counters_)
        {
            counter = 0;
        }
    }
};

enum class internal_statistics_id: uint32_t
{
    backend_input_pixels = 0,
    count
};

struct internal_statistics
{
    uint64_t backend_input_pixels;
};

class async_internal_statistics: public async_object
{
public:
    template <internal_statistics_id StatID>
    static void accumulate(async_object* query_obj, uint64_t v)
    {
        assert(dynamic_cast<async_internal_statistics*>(query_obj) != nullptr);
	    static_cast<async_internal_statistics*>(query_obj)->counters_[static_cast<uint32_t>(StatID)] += v;
    }

    virtual async_object_ids id()
    {
        return async_object_ids::internal_statistics;
    }

protected:
    boost::array<
        boost::atomic<uint64_t>,
        static_cast<uint32_t>(internal_statistics_id::count)
    > counters_;

    void get_value(void* v)
    {
        auto ret = reinterpret_cast<internal_statistics*>(v);
        ret->backend_input_pixels = counters_[static_cast<uint32_t>(internal_statistics_id::backend_input_pixels)];
    }

    virtual void init_async_data()
    {
        for(auto& counter: counters_)
        {
            counter = 0;
        }
    }
};

struct pipeline_profiles
{
	uint64_t gather_vtx;	// Including: Genenrate index of primitives and unique indexes
	uint64_t vtx_proc;
	uint64_t clipping;
	uint64_t vp_trans;
	uint64_t tri_dispatch;
	uint64_t ras;
};

enum class pipeline_profile_id: uint32_t
{
	gather_vtx = 0,
	vtx_proc,
	clipping,
	vp_trans,
	tri_dispatch,
	ras,
	count
};

class async_pipeline_profiles: public async_object
{
public:
	static uint64_t time_stamp()
	{
		using namespace boost::chrono;
		return static_cast<uint64_t>( duration_cast<nanoseconds>( high_resolution_clock::now().time_since_epoch() ).count() );
	}

    template <pipeline_profile_id StatID>
    static void accumulate(async_object* query_obj, uint64_t v)
    {
        assert(dynamic_cast<async_pipeline_profiles*>(query_obj) != nullptr);
	    static_cast<async_pipeline_profiles*>(query_obj)->counters_[static_cast<uint32_t>(StatID)] += v;
    }

    virtual async_object_ids id()
    {
		return async_object_ids::pipeline_profiles;
    }

protected:
    boost::array<
        boost::atomic<uint64_t>,
        static_cast<uint32_t>(pipeline_profile_id::count)
    > counters_;

    void get_value(void* v)
    {
        auto ret = reinterpret_cast<pipeline_profiles*>(v);
		ret->gather_vtx		 = counters_[static_cast<uint32_t>(pipeline_profile_id::gather_vtx)];
		ret->clipping		 = counters_[static_cast<uint32_t>(pipeline_profile_id::clipping)];
		ret->vp_trans		 = counters_[static_cast<uint32_t>(pipeline_profile_id::vp_trans)];
		ret->tri_dispatch	 = counters_[static_cast<uint32_t>(pipeline_profile_id::tri_dispatch)];
		ret->ras			 = counters_[static_cast<uint32_t>(pipeline_profile_id::ras)];
		ret->vtx_proc		 = counters_[static_cast<uint32_t>(pipeline_profile_id::vtx_proc)];
    }

    virtual void init_async_data()
    {
        for(auto& counter: counters_)
        {
            counter = 0;
        }
    }
};

struct time_stamp_fn
{
	typedef uint64_t (*type)();
	static uint64_t null()
	{
		return 0;
	}
};

template <typename ValueT>
struct accumulate_fn
{
    typedef void (*type)(async_object*, ValueT);
    static void null(async_object*, ValueT)
    {
    }
};

END_NS_SALVIAR();
