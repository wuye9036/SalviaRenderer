#include <eflib/include/platform/config.h>

#include <salviar/include/vertex_cache.h>
#include <salviar/include/stream_assembler.h>

#include <salviar/include/host.h>
#include <salviar/include/shader.h>
#include <salviar/include/shaderregs.h>
#include <salviar/include/shaderregs_op.h>
#include <salviar/include/renderer_impl.h>
#include <salviar/include/stream_assembler.h>
#include <salviar/include/thread_pool.h>

#include <salviar/include/shader_unit.h>

#include <eflib/include/platform/cpuinfo.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/ref.hpp>
#include <eflib/include/platform/boost_end.h>

using eflib::num_available_threads;

using boost::atomic;
using boost::shared_array;

using std::vector;

BEGIN_NS_SALVIAR();

const int GENERATE_INDICES_PACKAGE_SIZE = 8;
const int TRANSFORM_VERTEX_PACKAGE_SIZE = 8;

const size_t invalid_id = 0xffffffff;

class default_vertex_cache : public vertex_cache
{
public:
	default_vertex_cache(renderer_impl* owner)
		: owner_(owner)
		, verts_pool_( sizeof(vs_output) )
	{
	}

	void update_index_buffer(
		h_buffer const& index_buffer, format index_format,
		primitive_topology topology, uint32_t start_pos, uint32_t base_vert)
	{
		transformed_verts_.reset();
		topology_	= topology;
		index_fetcher_.initialize(index_buffer, index_format, topology, start_pos, base_vert);
	}

	void transform_vertices(uint32_t prim_count)
	{
		assembler_	= owner_->get_assembler().get();
		cpp_vs_		= owner_->get_vertex_shader().get();
		viewport_	= &(owner_->get_viewport());

		
		uint32_t prim_size = 0;
		switch(topology_)
		{
		case primitive_line_list:
		case primitive_line_strip:
			prim_size = 2;
			break;

		case primitive_triangle_list:
		case primitive_triangle_strip:
			prim_size = 3;
			break;
		}

		indices_.resize(prim_count * prim_size);

		atomic<int32_t> working_package(0);
		size_t num_threads = num_available_threads( );

		// Generate indices
		boost::function<void()> task_generate_indices = 
			boost::bind( &default_vertex_cache::generate_indices, this,
			boost::ref(indices_), static_cast<int32_t>(prim_count),
			prim_size, boost::ref(working_package), GENERATE_INDICES_PACKAGE_SIZE
			);
		for (size_t i = 0; i < num_threads - 1; ++ i)
		{
			global_thread_pool().schedule(task_generate_indices);
		}
		task_generate_indices();
		global_thread_pool().wait();

		// Unique indices
		std::vector<uint32_t> unique_indices = indices_;
		std::sort(unique_indices.begin(), unique_indices.end());
		unique_indices.erase(std::unique(unique_indices.begin(), unique_indices.end()), unique_indices.end());
		transformed_verts_.reset(new vs_output[unique_indices.size()]);
		used_verts_.resize(assembler_->num_vertices());

		// Transform vertexes
		if( cpp_vs_ )
		{
			assembler_->update_register_map( cpp_vs_->get_register_map() );
		}

		working_package = 0;
		boost::function<void()> task_transform_vertex;
		if( owner_->get_vertex_shader() )
		{
			task_transform_vertex = boost::bind(
				&default_vertex_cache::transform_vertex_cppvs,
				this, boost::ref(unique_indices),
				static_cast<int32_t>(unique_indices.size()), boost::ref(working_package), TRANSFORM_VERTEX_PACKAGE_SIZE
				);
		}
		else if ( owner_->get_host() )
		{
			task_transform_vertex = boost::bind(
				&default_vertex_cache::transform_vertex_vs2,
				this, boost::ref(unique_indices),
				static_cast<int32_t>(unique_indices.size()), boost::ref(working_package), TRANSFORM_VERTEX_PACKAGE_SIZE
				);
		}
		else
		{
			task_transform_vertex = boost::bind(
				&default_vertex_cache::transform_vertex_vs,
				this, boost::ref(unique_indices),
				static_cast<int32_t>(unique_indices.size()), boost::ref(working_package), TRANSFORM_VERTEX_PACKAGE_SIZE
				);
		}

		for (size_t i = 0; i < num_threads - 1; ++ i)
		{
			global_thread_pool().schedule(task_transform_vertex);
		}
		task_transform_vertex();
		global_thread_pool().wait();
	}

	vs_output& fetch(cache_entry_index id)
	{
		static vs_output null_obj;
		id = indices_[id];

#if defined(EFLIB_DEBUG)
		if((id > used_verts_.size()) || (-1 == used_verts_[id]))
		{
			assert( !"The vertex could not be transformed. Maybe errors occurred on index statistics or vertex tranformation." );
			return null_obj;
		}
#endif

		return transformed_verts_[used_verts_[id]];
	}

private:
	void generate_indices(
		vector<uint32_t>& indices, int32_t prim_count,
		uint32_t stride, atomic<int32_t>& working_package, int32_t package_size)
	{
		const int32_t num_packages = (prim_count + package_size - 1) / package_size;

		int32_t local_working_package = working_package ++;
		while (local_working_package < num_packages)
		{
			const int32_t start = local_working_package * package_size;
			const int32_t end = std::min(prim_count, start + package_size);
			for (int32_t i = start; i < end; ++ i){
				index_fetcher_.fetch_indices(&indices[i*stride], i);
			}

			local_working_package = working_package++;
		}
	}

	void transform_vertex_cppvs(
		vector<uint32_t> const& indices, int32_t index_count,
		atomic<int32_t>& working_package, int32_t package_size)
	{
		const int32_t num_packages = (index_count + package_size - 1) / package_size;

		int32_t local_working_package = working_package ++;
		while (local_working_package < num_packages)
		{
			const int32_t start = local_working_package * package_size;
			const int32_t end = std::min(index_count, start + package_size);
			for (int32_t i = start; i < end; ++ i){
				uint32_t id = indices[i];
				used_verts_[id] = i;

				vs_input vertex;
				assembler_->fetch_vertex(vertex, id);
				cpp_vs_->execute(vertex, transformed_verts_[i]);
			}

			local_working_package = working_package ++;
		}
	}

	void transform_vertex_vs(
		vector<uint32_t> const& indices, int32_t index_count,
		atomic<int32_t>& working_package, int32_t package_size )
	{
		vertex_shader_unit vsu	= *(owner_->vs_proto());

		vsu.bind_streams(assembler_);

		const int32_t num_packages = (index_count + package_size - 1) / package_size;

		int32_t local_working_package = working_package ++;
		while (local_working_package < num_packages)
		{
			const int32_t start = local_working_package * package_size;
			const int32_t end = std::min(index_count, start+package_size);
			for (int32_t i = start; i < end; ++ i)
			{
				uint32_t id = indices[i];
				used_verts_[id] = i;

				vsu.update( id );
				vsu.execute( transformed_verts_[i] );
			}

			local_working_package = working_package ++;
		}
	}

	void transform_vertex_vs2(
		vector<uint32_t> const& indices, int32_t index_count,
		atomic<int32_t>& working_package, int32_t package_size )
	{
		vx_shader_unit_ptr vsu	= owner_->get_host()->get_vx_shader_unit();

		const int32_t num_packages = (index_count + package_size - 1) / package_size;

		int32_t local_working_package = working_package ++;
		while (local_working_package < num_packages)
		{
			const int32_t start = local_working_package * package_size;
			const int32_t end = std::min(index_count, start+package_size);
			for (int32_t i = start; i < end; ++ i)
			{
				uint32_t vert_index = indices[i];
				used_verts_[vert_index] = i;

				vsu->execute(vert_index, transformed_verts_[i]);
			}

			local_working_package = working_package ++;
		}
	}
private:
	renderer_impl*			owner_;

	stream_assembler*		assembler_;
	vertex_shader*			cpp_vs_;
	viewport const*			viewport_;

	vector<uint32_t>		indices_;
	primitive_topology		topology_;
	index_fetcher			index_fetcher_;

	shared_array<vs_output> transformed_verts_;
	vector<int32_t>			used_verts_;

	boost::pool<>			verts_pool_;
};

vertex_cache_ptr create_default_vertex_cache(renderer_impl* owner)
{
	return vertex_cache_ptr( new default_vertex_cache(owner) );
}

END_NS_SALVIAR();
