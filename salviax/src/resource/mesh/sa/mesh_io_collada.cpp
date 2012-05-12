#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>

#include <salviax/include/resource/mesh/sa/collada.h>
#include <salviax/include/resource/mesh/sa/mesh.h>

#include <salviar/include/buffer.h>
#include <salviar/include/renderer.h>
#include <salviar/include/shader_abi.h>
#include <salviar/include/input_layout.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>
#include <sstream>
#include <set>

typedef salviar::format color_formats;

using salviar::renderer;
using salviar::h_buffer;
using salviar::language_value_types;
using salviar::input_element_desc;
using salviar::input_per_vertex;

using boost::property_tree::ptree;
using boost::property_tree::read_xml;
using boost::optional;
using boost::make_shared;
using boost::unordered_map;

using std::vector;
using std::string;
using std::fstream;
using std::make_pair;
using std::set;

BEGIN_NS_SALVIAX_RESOURCE();

template <typename T>
bool repack_data(void* dst, size_t dst_stride, void const* src, size_t stride, size_t count, size_t offset, vector<int> const& pattern)
{
	T const*	psrc = static_cast<T const*>(src);
	T*			pdst = static_cast<T*>(dst);

	for( int i = 0; i < count; ++i )
	{
		for( size_t i_scalar = 0; i_scalar < pattern.size(); ++i_scalar )
		{
			if (pattern[i_scalar] == -1){ continue; }
			size_t src_scalar_index = offset+i*stride+i_scalar;
			size_t dst_element_offset = static_cast<size_t>(pattern[i_scalar]);
			size_t dst_scalar_index = i*dst_stride+dst_element_offset;
			pdst[dst_scalar_index] = psrc[src_scalar_index];
		}
	}

	return true;
}

vector<char> repack_source(salviar::format& fmt, size_t& stride_in_bytes, dae_source* s)
{
	fmt = salviar::format_unknown;
	dae_accessor_ptr accessor = s->tech->accessor; 

	size_t count	= accessor->count;
	size_t stride	= accessor->stride ? accessor->stride : accessor->params.size();
	size_t offset	= accessor->offset;

	vector<int> pattern;
	BOOST_FOREACH( dae_param_ptr const& par, accessor->params )
	{
		pattern.push_back( par->index_xyzw_stpq() );
	}
	
	int max_index = *max_element( pattern.begin(), pattern.end() ) ;
	if( max_index == -1 ){ return vector<char>(); }
	size_t dst_stride = static_cast<size_t>(max_index+1);
	stride_in_bytes = dst_stride * s->arr->element_size();
	size_t byte_count = count * stride_in_bytes;

	vector<char> ret(byte_count);

	switch( s->arr->array_type )
	{
	case dae_array::float_array:
		repack_data<float>( (&ret[0]), dst_stride, &(s->arr->float_arr[0]), stride, count, offset, pattern);
		switch( dst_stride )
		{
		case 1:
			fmt = salviar::format_r32_float;
			break;
		case 2:
			fmt = salviar::format_r32g32_float;
			break;
		case 3:
			fmt = salviar::format_r32g32b32_float;
			break;
		case 4:
			fmt = salviar::format_r32g32b32a32_float;
			break;
		}
		break;
	case dae_array::int_array:
		repack_data<float>( (&ret[0]), dst_stride, &(s->arr->int_arr[0]), stride, count, offset, pattern );
		switch( dst_stride )
		{
		case 1:
			fmt = salviar::format_r32_sint;
			break;
		case 2:
			fmt = salviar::format_r32g32_sint;
			break;
		case 3:
			fmt = salviar::format_r32g32b32_sint;
			break;
		case 4:
			fmt = salviar::format_r32g32b32a32_sint;
			break;
		}
		break;
	}

	return ret;
}

void transfer_element( vector<char>& out_buffer, vector<char> const& in_buffer, size_t i_element, size_t offset_in_bytes, size_t stride_in_bytes )
{
	vector<char>::const_iterator src_beg_it = in_buffer.begin() + (offset_in_bytes+i_element*stride_in_bytes);
	vector<char>::const_iterator src_end_it = src_beg_it + stride_in_bytes;
	out_buffer.insert( out_buffer.end(), src_beg_it, src_end_it );
}

struct vertex_index_group
{
	vertex_index_group(){}
	vertex_index_group( size_t sz ): indexes(sz){}

	vector<uint32_t> indexes;	
};

bool operator == (vertex_index_group const& lhs, vertex_index_group const& rhs)
{
	return
		( lhs.indexes.size() == rhs.indexes.size() )
		&& std::equal( lhs.indexes.begin(), lhs.indexes.end(), rhs.indexes.begin() );
}

size_t hash_value( vertex_index_group const& v )
{
	size_t ret = 0;
	boost::hash_range( ret, v.indexes.begin(), v.indexes.end() );
	return ret;
}

vector<h_mesh> build_mesh( dae_mesh_ptr m, renderer* render )
{
	vector<h_mesh> ret;

	// Collect used sources.
	set<dae_source*> used_sources_set;
	BOOST_FOREACH( dae_triangles_ptr tri, m->triangle_sets )
	{
		BOOST_FOREACH( dae_input_ptr input, tri->inputs )
		{
			if( !input->source ) { continue; }
			
			dae_verts_ptr  pverts  = m->node_by_id<dae_verts>( *input->source );
			if( pverts ) {
				BOOST_FOREACH( dae_input_ptr const& input, pverts->inputs )
				{
					if( !input->source ) { continue; }
					dae_source* psource = pverts->node_by_id<dae_source>(*input->source).get();
					if( psource ){ used_sources_set.insert(psource); }
				}
			}
			
			dae_source_ptr psource = m->node_by_id<dae_source>( *input->source );
			if( psource )
			{
				used_sources_set.insert( psource.get() ); 
			}
		}
	}

	// Repack data source, adjust scalar orders, discard unused data.
	unordered_map<dae_source*, vector<char> >	repacked_sources;
	unordered_map<dae_source*, salviar::format>	fmts;
	unordered_map<dae_source*, size_t>			strides;

	BOOST_FOREACH( dae_source* src, used_sources_set )
	{
		salviar::format fmt = salviar::format_unknown;
		size_t stride = 0;

		repacked_sources[src] = repack_source(fmt, stride, src);
		fmts[src] = fmt;
		strides[src] = stride;
	}

	// Build Triangles
	BOOST_FOREACH( dae_triangles_ptr const& triangles, m->triangle_sets )
	{
		// Extract vertices to sources.
		vector<dae_input_ptr>	inputs;
		vector<size_t>			offsets;
		size_t					index_stride = triangles->inputs.size();
		BOOST_FOREACH( dae_input_ptr const& input, triangles->inputs )
		{
			dae_node_ptr input_source = input->source_node();
			if( input->semantic
				&& *input->semantic == "VERTEX"
				&& input_source->is_a<dae_verts>() )
			{
				dae_verts* verts = input_source->as<dae_verts>();
				inputs.insert( inputs.end(), verts->inputs.begin(), verts->inputs.end() );
				offsets.insert( offsets.end(), verts->inputs.size(), input->offset );
			}
			else
			{
				inputs.push_back(input);
				offsets.push_back(input->offset);
			}
		}
		size_t index_group_size = inputs.size();

		// Merge per-input index to an unified index, and re-order the data to matching unified index.
		typedef unordered_map<vertex_index_group, uint32_t> index_dict_t;
		vector< vector<char> >	buffers_data(index_group_size);
		vector<uint32_t>		attribute_merged_indexes;
		index_dict_t			index_dict; // a map from grouped index to merged index.
		size_t					vertex_index = 0;
		vertex_index_group		index_group( index_group_size );

		for( size_t i_tri_vert = 0; i_tri_vert < triangles->count*3; ++i_tri_vert ){
			// make index group
			for( size_t i_comp = 0; i_comp < index_group_size; ++i_comp ){
				index_group.indexes[i_comp] = (uint32_t)triangles->indexes[ i_tri_vert*index_stride+offsets[i_comp] ];
			}

			index_dict_t::iterator it = index_dict.find(index_group);
			if( it != index_dict.end() ) {
				attribute_merged_indexes.push_back(it->second);
			} else {
				// Copy data to buffer.
				for( size_t i_comp = 0; i_comp < index_group_size; ++i_comp )
				{
					dae_source* source_of_input = inputs[i_comp]->source_node()->as<dae_source>();
					assert( source_of_input );

					vector<char>&	repacked_source_data( repacked_sources[source_of_input] );
					size_t			stride_in_bytes = strides[source_of_input];
					transfer_element( buffers_data[i_comp], repacked_source_data, index_group.indexes[i_comp], 0, stride_in_bytes );
				}

				attribute_merged_indexes.push_back(vertex_index);
				index_dict[index_group] = vertex_index;
				++vertex_index;
			}
		}

		// Build vertex buffer and input description
		vector<h_buffer>			buffers;
		vector<size_t>				buffer_strides;
		vector<input_element_desc>	input_descs;

		for( int i_source = 0; i_source < inputs.size(); ++i_source )
		{
			dae_input*  input = inputs[i_source].get();
			dae_source* input_source = input->source_node()->as<dae_source>();

			input_descs.push_back( input_element_desc() );
			if( input->semantic ){
				input_descs.back().semantic_name = (*input->semantic).c_str();
			}
			input_descs.back().semantic_index	= static_cast<uint32_t>(input->set);
			input_descs.back().data_format		= fmts[input_source];
			input_descs.back().input_slot		= buffers.size();
			input_descs.back().slot_class		= input_per_vertex; 
			input_descs.back().aligned_byte_offset = 0;

			size_t buffer_data_size = buffers_data[i_source].size();
			h_buffer buf = render->create_buffer(buffer_data_size);
			buf->transfer(0, &(buffers_data[i_source][0]), buffer_data_size, buffer_data_size, buffer_data_size, 1 );
			buffers.push_back( buf );
			buffer_strides.push_back( strides[input_source] );
		}

		mesh* pmesh = new mesh(render);
		for( size_t i = 0; i < buffers.size(); ++i )
		{
			pmesh->add_vertex_buffer(i, buffers[i], buffer_strides[i], 0);
		}
		pmesh->set_input_element_descs( input_descs );

		// Build Index Buffer and Topo
		size_t index_size = attribute_merged_indexes.size();
		size_t index_buffer_size = index_size * 4;
		h_buffer index_buf = render->create_buffer(index_buffer_size);
		index_buf->transfer(0, &(attribute_merged_indexes[0]), index_buffer_size, index_buffer_size, index_buffer_size, 1 );
		
		pmesh->set_index_buffer(index_buf);
		pmesh->set_index_type(salviar::format_r32_uint);
		pmesh->set_primitive_count(index_size/3);

		ret.push_back( h_mesh(pmesh) );
	}

	return ret;
}

vector<h_mesh> create_mesh_from_collada( renderer* render, std::string const& file_name )
{
	vector<h_mesh> ret;
	fstream fstr( file_name, std::ios::in );
	if( !fstr.is_open() ) { return ret; }
	ptree dae_doc;
	read_xml( fstr, dae_doc );
	fstr.close();

	optional<ptree&> collada_root = dae_doc.get_child_optional( "COLLADA" );
	if( !collada_root ) return ret;

	optional<ptree&> geometries_root = collada_root->get_child_optional( "library_geometries" );
	if( !geometries_root ) return ret;

	dae_dom_ptr pdom = make_shared<dae_dom>();

	BOOST_FOREACH( ptree::value_type& geom_child, geometries_root.get() )
	{
		if( geom_child.first == "geometry" )
		{
			BOOST_FOREACH( ptree::value_type& mesh_child, geom_child.second )
			{
				if( mesh_child.first == "mesh")
				{
					dae_mesh_ptr dae_mesh_node = dae_mesh::parse( mesh_child.second, pdom );
					if( !dae_mesh_node ) return ret;
					return build_mesh( dae_mesh_node, render );
				}
			}
		}
	}

	return ret;
}

END_NS_SALVIAX_RESOURCE();
