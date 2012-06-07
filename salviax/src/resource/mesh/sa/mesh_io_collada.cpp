#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>

#include <salviax/include/resource/mesh/sa/collada.h>
#include <salviax/include/resource/mesh/sa/mesh_impl.h>
#include <salviax/include/resource/mesh/sa/skin_mesh_impl.h>

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
using salviar::semantic_value;

using eflib::vec4;
using eflib::int4;
using eflib::mat44;

using boost::property_tree::ptree;
using boost::property_tree::read_xml;
using boost::optional;
using boost::make_shared;
using boost::unordered_map;
using boost::shared_ptr;
using boost::shared_polymorphic_cast;

using std::vector;
using std::string;
using std::fstream;
using std::make_pair;
using std::set;
using std::pair;

BEGIN_NS_SALVIAX_RESOURCE();

EFLIB_DECLARE_CLASS_SHARED_PTR(mesh_impl);

struct skin_info
{
	string				source;

	mat44				bind_matrix;
	vector<string>		joints;
	vector<mat44>		joint_inv_matrix;
	vector<float>		weights;

	vector<uint32_t>	vertex_skin_info_start_pos;
	vector<uint32_t>	vertex_skin_info_count;
	vector<
		pair<uint32_t /*skin_info.joint*/, uint32_t /*skin_info.weight*/>
	>					vertex_skin_infos; 
};

typedef shared_ptr<skin_info> skin_info_ptr;

template <typename T>
bool repack_data(void* dst, size_t dst_stride, void const* src, size_t stride, size_t count, size_t offset, vector<int> const& pattern)
{
	T const*	psrc = static_cast<T const*>(src);
	T*			pdst = static_cast<T*>(dst);

	for( size_t i = 0; i < count; ++i )
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

vector<h_mesh> build_mesh( dae_mesh_ptr m, skin_info* skinfo, renderer* render )
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
		size_t					vertex_index_offset = 0;
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
				vertex_index_offset = input->offset;
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
		vector< vector<char> >	buffers_data(skinfo ? index_group_size + 2 : index_group_size ); // If skinfo is available, last two buffers for joints and weights.
		vector<uint32_t>		attribute_merged_indexes;
		index_dict_t			index_dict; // a map from grouped index to merged index.
		size_t					merged_vertex_counter = 0;
		vertex_index_group		index_group(index_group_size);

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

				// Copy joint id and weights to buffer
				if( skinfo )
				{
					size_t vert_index = (uint32_t)triangles->indexes[i_tri_vert*index_stride+vertex_index_offset];
					size_t vert_skin_info_start  = skinfo->vertex_skin_info_start_pos[vert_index];
					size_t vert_skin_info_length = skinfo->vertex_skin_info_count[vert_index]; 
					int4 vert_joint_ids(-1, -1, -1, -1);
					vec4 vert_joint_weights(0.0f, 0.0f, 0.0f, 0.0f);
					for( size_t i = 0; i < vert_skin_info_length; ++i )
					{
						if( i > 3 ){ break; }
						size_t skin_info_index = vert_skin_info_start + i;
						vert_joint_ids.data_[i] = skinfo->vertex_skin_infos[skin_info_index].first;
						vert_joint_weights.data_[i] = skinfo->weights[skinfo->vertex_skin_infos[skin_info_index].second];
					}
					vector<char>& weights_buffer = buffers_data.back();
					vector<char>& joint_buffer = *(buffers_data.end()-2);
					joint_buffer.insert(
						joint_buffer.end(), (char*)(&vert_joint_ids), (char*)(&vert_joint_ids) + sizeof(vert_joint_ids)
						);
					weights_buffer.insert(
						weights_buffer.end(), (char*)(&vert_joint_weights), (char*)(&vert_joint_weights) + sizeof(vert_joint_weights)
						);
				}

				attribute_merged_indexes.push_back(merged_vertex_counter);
				index_dict[index_group] = merged_vertex_counter;
				++merged_vertex_counter;
			}
		}

		// Build vertex buffer and input description
		vector<h_buffer>			buffers;
		vector<size_t>				buffer_strides;
		vector<input_element_desc>	input_descs;

		for( size_t i_source = 0; i_source < inputs.size(); ++i_source )
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

		// Add skin info buffers.
		if(skinfo)
		{
			// Add blend indices buffer.
			input_descs.push_back( input_element_desc() );
			input_descs.back().semantic_name	= "BLEND_INDICES";
			input_descs.back().semantic_index	= 0;
			input_descs.back().data_format		= salviar::format_r32g32b32a32_sint;
			input_descs.back().input_slot		= buffers.size();
			input_descs.back().slot_class		= input_per_vertex; 
			input_descs.back().aligned_byte_offset = 0;

			vector<char>& joint_indices_buffer = buffers_data[buffers_data.size()-2];
			size_t buffer_data_size = joint_indices_buffer.size();
			h_buffer buf = render->create_buffer(buffer_data_size);
			buf->transfer(0, &(joint_indices_buffer[0]), buffer_data_size, buffer_data_size, buffer_data_size, 1 );
			buffers.push_back( buf );
			buffer_strides.push_back( sizeof(int4) );

			// Add blend weights buffer
			input_descs.push_back( input_element_desc() );
			input_descs.back().semantic_name	= "BLEND_WEIGHTS";
			input_descs.back().semantic_index	= 0;
			input_descs.back().data_format		= salviar::format_r32g32b32a32_float;
			input_descs.back().input_slot		= buffers.size();
			input_descs.back().slot_class		= input_per_vertex; 
			input_descs.back().aligned_byte_offset = 0;

			vector<char>& joint_weights_buffer = buffers_data.back();
			buffer_data_size = joint_indices_buffer.size();
			buf = render->create_buffer(buffer_data_size);
			buf->transfer(0, &(joint_weights_buffer[0]), buffer_data_size, buffer_data_size, buffer_data_size, 1 );
			buffers.push_back( buf );
			buffer_strides.push_back( sizeof(int4) );
		}

		mesh_impl* pmesh = new mesh_impl(render);
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

skin_info_ptr build_skin_info( dae_skin_ptr skin )
{
	skin_info_ptr ret = make_shared<skin_info>();

	ret->bind_matrix = skin->bind_shape_mat;
	ret->source = skin->unqualified_source_name();

	BOOST_FOREACH( dae_input_ptr const& joint_fmt, skin->joint_formats )
	{
		if(!joint_fmt->semantic) { continue; }
		if(*(joint_fmt->semantic) == "JOINT"){
			dae_source* joint_name_source = joint_fmt->source_node()->as<dae_source>();
			assert( joint_name_source );
			assert( joint_name_source->arr->array_type == dae_array::name_array );
			ret->joints = joint_name_source->arr->name_arr;	// Copy directly.
		}
		else if(*(joint_fmt->semantic) == "INV_BIND_MATRIX")
		{
			dae_source* inv_bind_matrix_source = joint_fmt->source_node()->as<dae_source>();
			assert( inv_bind_matrix_source );
			assert( inv_bind_matrix_source->arr->array_type == dae_array::float_array );

			dae_accessor_ptr accessor = inv_bind_matrix_source->tech->accessor;
			assert( accessor->params[0]->vtype == salviar::lvt_f32m44 );

			for( size_t i = 0; i < accessor->count; ++i )
			{
				size_t start_addr = i*accessor->stride;
				float* mat_addr = &( (inv_bind_matrix_source->arr->float_arr)[start_addr] );
				ret->joint_inv_matrix.push_back( *reinterpret_cast<mat44*>(mat_addr) );
			}
		}
	}

	size_t joint_offset = 0;
	size_t weight_offset = 0;
	size_t weight_index_stride = 0;
	BOOST_FOREACH( dae_input_ptr const& weight_input, skin->weights->inputs )
	{
		if(!weight_input->semantic) { continue; }
		if( *weight_input->semantic == "JOINT" )
		{
			joint_offset = weight_input->offset;
			++weight_index_stride;
		}
		else if( *weight_input->semantic == "WEIGHT" )
		{
			weight_offset = weight_input->offset;
			dae_source* weight_source = weight_input->source_node()->as<dae_source>();
			assert(weight_source);
			ret->weights = weight_source->arr->float_arr; // Copy directly. maybe we need to repack.
			++weight_index_stride;
		}
	}
	assert( weight_index_stride == 2 );

	size_t vertex_skin_info_cursor = 0;
	for( size_t i_vert = 0; i_vert < skin->weights->count; ++i_vert )
	{
		size_t vcount = skin->weights->vcount[i_vert];
		ret->vertex_skin_info_start_pos.push_back(vertex_skin_info_cursor);
		ret->vertex_skin_info_count.push_back(vcount);
		for( size_t i_joint = 0; i_joint < vcount; ++i_joint )
		{
			size_t pos_of_vert_skin_info = (vertex_skin_info_cursor+i_joint)*weight_index_stride;
			size_t pos_of_joint_id = pos_of_vert_skin_info + joint_offset;
			size_t pos_of_weight = pos_of_vert_skin_info + weight_offset;
			ret->vertex_skin_infos.push_back( make_pair(skin->weights->v[pos_of_joint_id], skin->weights->v[pos_of_weight]) );
		}
		vertex_skin_info_cursor += vcount;
	}

	return ret;
}

scene_node_ptr build_scene_node(
	dae_scene_node_ptr scene,
	unordered_map<dae_scene_node_ptr,scene_node_ptr>& dae_node_to_joints,
	unordered_map<dae_matrix_ptr, mat44*>&	dae_node_to_matrix)
{
	scene_node_ptr ret = boost::make_shared<scene_node>( (scene_node*)NULL, std::string() );
	ret->name = *(scene->id);
	dae_node_to_joints.insert( make_pair(scene, ret) );

	BOOST_FOREACH(dae_scene_node_ptr const& child, scene->children){
		scene_node_ptr child_scene = build_scene_node(child, dae_node_to_joints, dae_node_to_matrix);
		child_scene->parent = ret.get();
		ret->children.push_back(child_scene);
		
	}

	if(scene->mat)
	{
		ret->local_matrix = ret->original_matrix = scene->mat->mat;
		dae_node_to_matrix.insert( make_pair( scene->mat, &(ret->local_matrix) ) );
	}

	return ret;
}

void assign_mat44(mat44* lhs, mat44 const& rhs)
{
	*lhs = rhs;
}

vector<animation_player_ptr> build_animations(
	dae_animations_ptr animations,
	unordered_map<dae_matrix_ptr, mat44*>&	dae_node_to_matrix
	)
{
	vector<animation_player_ptr> ret;

	BOOST_FOREACH(dae_animation_ptr const& anim, animations->anims)
	{
		// Get channel
		dae_sampler* samp = anim->channel->source_node()->as<dae_sampler>();
		assert(samp);
		if( !anim->channel->target ) continue;
		dae_node_ptr target_node = animations->owner->node_by_path(*anim->channel->target);
		assert(target_node);
		
		dae_source* input_source = samp->data_in->source_node()->as<dae_source>();
		assert(input_source);
		dae_source* output_source = samp->data_out->source_node()->as<dae_source>();
		assert(output_source);
		dae_source* interp_source = samp->interpolation->source_node()->as<dae_source>();

		animation_player_ptr anim_player;
		if( target_node->is_a<dae_matrix>() )
		{
			shared_ptr< animation_player_impl<mat44> > anim_player_mat44
				= make_shared< animation_player_impl<mat44> >();
			anim_player = anim_player_mat44;

			anim_player_mat44->anim_info2()->X = input_source->arr->float_arr;

			for( size_t i = 0; i < output_source->arr->float_arr.size()/16; ++i)
			{
				float* pfloat = &(output_source->arr->float_arr[i*16]);
				mat44* pmat = reinterpret_cast<mat44*>(pfloat);
				anim_player_mat44->anim_info2()->Y.push_back(*pmat); 
			}
			
			mat44* target_data = dae_node_to_matrix[shared_polymorphic_cast<dae_matrix>(target_node)];
			anim_player_mat44->anim_info2()->applier = boost::bind( assign_mat44, target_data, _1 );
		}

		if(anim_player)
		{
			for(size_t i = 0; i < interp_source->arr->name_arr.size(); ++i)
			{
				interpolation_methods im = im_none;
				if(interp_source->arr->name_arr[i] == "LINEAR") {
					im = im_linear;
				}
				anim_player->anim_info()->interps.push_back(im);
			}
		}

		ret.push_back(anim_player);
	}

	return ret;
}

skin_mesh_ptr create_mesh_from_collada( renderer* render, std::string const& file_name )
{
	skin_mesh_impl_ptr ret = make_shared<skin_mesh_impl>();

	fstream fstr( file_name, std::ios::in );
	if( !fstr.is_open() ) { return ret; }
	ptree dae_doc;
	read_xml( fstr, dae_doc );
	fstr.close();

	optional<ptree&> collada_root = dae_doc.get_child_optional( "COLLADA" );
	if( !collada_root ) return skin_mesh_impl_ptr();

	dae_dom_ptr pdom = make_shared<dae_dom>();

	optional<ptree&> geometries_root = collada_root->get_child_optional( "library_geometries" );
	if( !geometries_root ) return ret;
	optional<ptree&> controllers_root = collada_root->get_child_optional( "library_controllers" );
	if( !controllers_root ) return ret;
	optional<ptree&> animations_root = collada_root->get_child_optional( "library_animations" );
	if( !animations_root ) return ret;
	optional<ptree&> scenes_root = collada_root->get_child_optional("library_visual_scenes");
	if( !scenes_root ) return ret;
	
	// Build skin infos.
	unordered_map<string, skin_info_ptr> skin_infos;
	BOOST_FOREACH( ptree::value_type& ctrl_child, controllers_root.get() ){
		if( ctrl_child.first == "controller" ){
			dae_controller_ptr ctrl_node = pdom->load_node<dae_controller>(ctrl_child.second, NULL);
			skin_info_ptr skinfo = build_skin_info(ctrl_node->skin);
			skin_infos[skinfo->source] = skinfo;
		}
	}

	// Build mesh.
	BOOST_FOREACH( ptree::value_type& geom_child, geometries_root.get() )
	{
		if( geom_child.first == "geometry" )
		{
			string geom_id = geom_child.second.get<string>("<xmlattr>.id");
			skin_info* skinfo = 
				skin_infos.count(geom_id) > 0 ? skin_infos[geom_id].get() : NULL;
			
			optional<ptree&> mesh_node = geom_child.second.get_child_optional("mesh");
			assert(mesh_node);

			dae_mesh_ptr dae_mesh_node = pdom->load_node<dae_mesh>(*mesh_node, NULL);
			if( !dae_mesh_node ) return skin_mesh_impl_ptr();

			ret->submeshes = build_mesh( dae_mesh_node, skinfo, render );
			ret->joints = skinfo->joints;
			ret->bind_inv_mats = skinfo->joint_inv_matrix;
		}
	}

	// Build scene hierarchy
	unordered_map<dae_scene_node_ptr, scene_node_ptr>	dae_scene_node_to_scene_node;
	unordered_map<dae_matrix_ptr, mat44*>				dae_matrix_to_matrix;
	{
		dae_visual_scenes_ptr scenes = pdom->load_node<dae_visual_scenes>(*scenes_root, NULL);
		BOOST_FOREACH( dae_scene_node_ptr const& child, scenes->scenes )
		{
			ret->roots.push_back( build_scene_node(child, dae_scene_node_to_scene_node, dae_matrix_to_matrix) );
		}

		typedef unordered_map<dae_scene_node_ptr, scene_node_ptr>::value_type item_type;
		BOOST_FOREACH(item_type const& item, dae_scene_node_to_scene_node)
		{
			if( item.first->id )
			{
				ret->joint_nodes.insert( make_pair(*item.first->id, item.second.get()) );
			}
		}
	}
	
	// Build animations
	{
		dae_animations_ptr anims_node = pdom->load_node<dae_animations>(*animations_root, NULL);
		ret->anims = build_animations(anims_node, dae_matrix_to_matrix);
	}
	
	return ret;
}

vector<h_mesh> build_mesh_from_file(renderer* render, std::string const& file_name)
{
	vector<h_mesh> ret;

	fstream fstr( file_name, std::ios::in );
	if( !fstr.is_open() ) { return ret; }
	ptree dae_doc;
	read_xml( fstr, dae_doc );
	fstr.close();

	optional<ptree&> collada_root = dae_doc.get_child_optional( "COLLADA" );
	if( !collada_root ) return ret;

	dae_dom_ptr pdom = make_shared<dae_dom>();

	optional<ptree&> geometries_root = collada_root->get_child_optional( "library_geometries" );
	if( !geometries_root ) return ret;

	// Build mesh.
	BOOST_FOREACH( ptree::value_type& geom_child, geometries_root.get() )
	{
		if( geom_child.first == "geometry" )
		{
			string geom_id = geom_child.second.get<string>("<xmlattr>.id");

			optional<ptree&> mesh_node = geom_child.second.get_child_optional("mesh");
			assert(mesh_node);

			dae_mesh_ptr dae_mesh_node = pdom->load_node<dae_mesh>(*mesh_node, NULL);
			if( !dae_mesh_node ) return ret;

			return build_mesh( dae_mesh_node, NULL, render );
		}
	}

	return ret;
}

void merge_buffer_to_mesh(
	mesh_impl* merged_mesh, /*OUT*/
	vector<input_element_desc>& ieds,
	size_t& slot_counter /*IN OUT*/,
	input_element_desc const& ied, mesh_impl* source_mesh, /*IN*/
	size_t src_mesh_idx, size_t src_mesh_count /*For compute merged semantic index*/)
{
	vector<size_t>::iterator slot_it = find( source_mesh->slots_.begin(), source_mesh->slots_.end(), ied.input_slot );
	size_t buffer_index = (size_t)distance(source_mesh->slots_.begin(), slot_it);
	merged_mesh->add_vertex_buffer(
		slot_counter,
		source_mesh->vertex_buffers_[buffer_index],
		source_mesh->strides_[buffer_index],
		source_mesh->offsets_[buffer_index]
	);
	
	ieds.push_back(ied);
	ieds.back().input_slot = slot_counter;
	ieds.back().semantic_index = src_mesh_idx+src_mesh_count*ied.semantic_index;

	++slot_counter;
}

h_mesh merge_mesh_for_morphing( h_mesh lm, h_mesh rm )
{
	mesh_impl* left_mesh  = dynamic_cast<mesh_impl*>( lm.get() );
	mesh_impl* right_mesh = dynamic_cast<mesh_impl*>( rm.get() );

	EFLIB_ASSERT_AND_IF( left_mesh && right_mesh, "Left mesh or right mesh is invalid." )
	{
		return h_mesh();
	}

	EFLIB_ASSERT_AND_IF( left_mesh->primcount_ == right_mesh->primcount_, "Primitives amount is not matched." )
	{
		return h_mesh();
	}

	EFLIB_ASSERT_AND_IF( left_mesh->index_fmt_ == right_mesh->index_fmt_, "Index format is not matched." )
	{
		return h_mesh();
	}

	mesh_impl_ptr ret = make_shared<mesh_impl>(left_mesh->device_);
	size_t slot_counter = 0;
	vector<input_element_desc> merged_mesh_ieds;

	BOOST_FOREACH( input_element_desc& left_ied, left_mesh->elem_descs_ )
	{
		semantic_value lsv(left_ied.semantic_name, left_ied.semantic_index);

		BOOST_FOREACH( input_element_desc& right_ied, right_mesh->elem_descs_ )
		{
			semantic_value rsv( right_ied.semantic_name, right_ied.semantic_index );
			if(lsv == rsv)
			{
				merge_buffer_to_mesh(ret.get(), merged_mesh_ieds, slot_counter, left_ied, left_mesh, 0, 2);
				merge_buffer_to_mesh(ret.get(), merged_mesh_ieds, slot_counter, right_ied, right_mesh, 1, 2);
				break;
			}
		}
	}

	ret->set_input_element_descs(merged_mesh_ieds);

	ret->set_attached_data( left_mesh->get_attached() );
	ret->set_index_type( left_mesh->index_fmt_ );
	ret->set_index_buffer( left_mesh->index_buffer_ );
	ret->set_primitive_count( left_mesh->primcount_ );

	return ret;
}

mesh_ptr create_morph_mesh_from_collada( salviar::renderer* render, std::string const& src, std::string const& dst )
{
	vector<h_mesh> src_meshes = build_mesh_from_file(render, src);
	vector<h_mesh> dst_meshes = build_mesh_from_file(render, dst);

	EFLIB_ASSERT_AND_IF( src_meshes.size() == dst_meshes.size(), "Morph meshes need to be combined from two meshes with same size." )
	{
		return mesh_ptr();
	}

	for(size_t i = 0; i < src_meshes.size(); ++i)
	{
		mesh_ptr ret = merge_mesh_for_morphing( src_meshes[i], dst_meshes[i] );
		if(ret) { return ret; }
	}

	return mesh_ptr();
}

END_NS_SALVIAX_RESOURCE();
