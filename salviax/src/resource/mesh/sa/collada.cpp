#include <salviax/include/resource/mesh/sa/collada.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#include <sstream>

using eflib::mat44;

using boost::property_tree::ptree;
using boost::make_shared;
using boost::unordered_map;
using boost::optional;

using std::vector;
using std::string;
using std::stringstream;

BEGIN_NS_SALVIAX_RESOURCE();

template <typename T>
void parse_array( vector<T>& arr, std::string const& content )
{
	stringstream ss(content);
	T tmp;
	while( ss >> tmp ){ arr.push_back(tmp); }
}

template <typename IteratorT>
void parse_array(IteratorT begin, IteratorT end, std::string const& content)
{
	IteratorT it = begin;
	stringstream ss(content);
	while( it != end && ss >> *it ){ ++it; }
}

void dae_node::parse_attribute(ptree& xml_node)
{
	id		= xml_node.get_optional<string>("<xmlattr>.id");
	sid		= xml_node.get_optional<string>("<xmlattr>.sid");
	name	= xml_node.get_optional<string>("<xmlattr>.name");
	source	= xml_node.get_optional<string>("<xmlattr>.source");
}

bool dae_tech::parse(ptree& root)
{
	optional<ptree&> accessor_xml_node = root.get_child_optional("accessor");
	if( !accessor_xml_node ) { return false; }
	accessor = load_child<dae_accessor>(*accessor_xml_node);
	return (bool)accessor;
}

bool dae_accessor::parse(ptree& root)
{
	source_array = owner->get_node<dae_array>(*source);

	if( !source_array ) { return false; }

	offset = root.get( "<xmlattr>.offset", size_t(0) );
	stride = root.get( "<xmlattr>.stride", size_t(0) );
	count  = root.get( "<xmlattr>.count" , size_t(0) );

	for(ptree::value_type& child: root)
	{
		if( child.first == "param" )
		{
			params.push_back( load_child<dae_param>(child.second) );
		}
	}

	return true;
}

bool dae_mesh::parse(ptree& root)
{
	for(ptree::value_type& child: root)
	{
		if( child.first == "<xmlattr>" ) { continue; }

		if( child.first == "source" ){
			sources.push_back( load_child<dae_source>(child.second) );
		} else if( child.first == "vertices" ) {
			verts.push_back( load_child<dae_verts>(child.second) );
		} else if( child.first == "triangles" ) {
			triangle_sets.push_back( load_child<dae_triangles>(child.second) );
		} else {
			assert(false);
		}
	}

	return true;
}

bool dae_source::parse(ptree& root)
{

	for(ptree::value_type& child: root)
	{
		if( child.first == "<xmlattr>" ) { continue; }

		if(/**/child.first == "float_array"
			|| child.first == "int_array"
			|| child.first == "Name_array" )
		{
			arr = load_child<dae_array>(child.second);
			arr->parse_content(child.first);
		}
		else if( child.first == "technique_common" )
		{
			tech = load_child<dae_tech>(child.second);
		}
		else
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	return true;
}

bool dae_verts::parse(ptree& root)
{
	for(ptree::value_type& child: root)
	{
		if( child.first == "<xmlattr>" )
		{
			count = child.second.get("count", (size_t)0);
			material_name = child.second.get_optional<string>("material");
		}
		else if( child.first == "input" )
		{
			inputs.push_back( load_child<dae_input>(child.second) );
		}
	}

	return true;
}

bool dae_triangles::parse(ptree& root)
{
	for(ptree::value_type& child: root)
	{
		if( child.first == "<xmlattr>" )
		{
			count = child.second.get("count", 0);
			material_name = child.second.get_optional<string>("material");
		}
		else if( child.first == "p" )
		{
			parse_array( indexes, child.second.get_value<string>() );
		}
		else if( child.first == "input" )
		{
			inputs.push_back( load_child<dae_input>(child.second) );
		}
	}

	return true;
}

bool dae_param::parse(ptree& root)
{
	string type_str = root.get<string>("<xmlattr>.type");
	
	stype = st_none;

	/**/ if(type_str == "float")	{ vtype = salviar::lvt_float; }
	else if(type_str == "int")		{ vtype = salviar::lvt_sint32;}
	else if(type_str == "float4x4") { vtype = salviar::lvt_f32m44;}
	else if(type_str == "name")		{ vtype = salviar::lvt_none; stype = st_name; }
	else { assert(false); }

	return true;
}

bool dae_input::parse(ptree& root)
{
	semantic= root.get_optional<string>("<xmlattr>.semantic");
	offset	= root.get("<xmlattr>.offset",	(size_t)0);
	set		= root.get("<xmlattr>.set",		(size_t)0);

	return true;
}

bool dae_param::index( string const& index_seq, size_t& idx )
{
	if( !name ) return false;
	idx = index_seq.find( (*name)[0] );
	return idx != string::npos;
}

int dae_param::index( string const& index_seq )
{
	size_t ret = 0;
	if( index(index_seq, ret) )
	{
		return static_cast<int>(ret);
	}
	return -1;
}

int dae_param::index_xyzw_stpq()
{
	int ret = index("XYZW");
	if( ret != -1 ) return ret;
	return index("STPQ");
}

bool dae_array::parse(ptree& root)
{
	count = root.get( "<xmlattr>.count", 0 );
	content = root.get_value_optional<string>();
	return true;
}

size_t dae_array::element_size()
{
	switch(array_type)
	{
	case float_array:
		return sizeof(float);
	case int_array:
		return sizeof(int);
	default:
		assert(false);
	}

	return 0;
}

void dae_array::parse_content( std::string const& tag_name )
{
	if( tag_name == "float_array" ) array_type = float_array;
	if( tag_name == "int_array" )	array_type = int_array;
	if( tag_name == "Name_array")	array_type = name_array;

	// Parse elements
	if( content )
	{
		switch (array_type)
		{
		case float_array:
			{
				parse_array(float_arr, *content );
				if( count == 0 ) { count = float_arr.size(); }
			}
			break;
		case int_array:
			{
				parse_array(int_arr, *content );
				if( count == 0 ) { count = float_arr.size(); }
			}
			break;
		case name_array:
			{
				parse_array(name_arr, *content );
				if( count == 0 ) { count = float_arr.size(); }
			}
			break;
		}
	}
}

bool dae_controller::parse(ptree& root)
{
	optional<ptree&> skin_node = root.get_child_optional("skin");
	assert( skin_node );
	skin = load_child<dae_skin>(*skin_node);
	return true;
}

bool dae_skin::parse(ptree& root)
{
	bind_shape_mat = mat44::identity();

	for(ptree::value_type& child: root)
	{
		if(child.first == "bind_shape_matrix")
		{
			stringstream ss( child.second.get_value<string>() );
			for(int i = 0; i < 16; ++i){
				ss >> bind_shape_mat.data_[i/4][i%4];
			}
		}
		else if(child.first == "source")
		{
			joint_sources.push_back( load_child<dae_source>(child.second) );
		}
		else if(child.first == "joints")
		{
			for(ptree::value_type& joint_input: child.second)
			{
				if(joint_input.first == "input")
				{
					joint_formats.push_back( load_child<dae_input>(joint_input.second) );
				}
			}
		}
		else if(child.first == "vertex_weights")
		{
			weights = load_child<dae_vertex_weights>(child.second);
		}
	}

	return true;
}

bool dae_vertex_weights::parse(ptree& root)
{
	count = root.get("<xmlattr>.count", (size_t)0);
	for(ptree::value_type& child: root)
	{
		if( child.first == "input" )
		{
			inputs.push_back( load_child<dae_input>(child.second) );
		}
		else if(child.first == "vcount")
		{
			parse_array( vcount, child.second.get_value<string>() );
		}
		else if(child.first == "v")
		{
			parse_array( v, child.second.get_value<string>() );
		}
	}

	return true;
}


bool dae_visual_scenes::parse(ptree& root)
{
	for(ptree::value_type& child: root)
	{
		if( child.first == "visual_scene" )
		{
			scenes.push_back( load_child<dae_scene_node>(child.second) );
		}
	}
	return true;
}

bool dae_scene_node::parse(ptree& root)
{
	type_name = root.get_optional<string>("<xmlattr>.type");

	for(ptree::value_type& child: root)
	{
		if( child.first == "matrix")
		{
			mat = load_child<dae_matrix>(child.second);
		}
		else if( child.first == "node" )
		{
			children.push_back( load_child<dae_scene_node>(child.second) );
		}
	}

	return true;
}

bool dae_animations::parse(ptree& root)
{
	for(ptree::value_type& child: root)
	{
		if(child.first == "animation")
		{
			anims.push_back( load_child<dae_animation>(child.second) );
		}
	}

	return true;
}


bool dae_animation::parse(ptree& root)
{
	ptree* real_root = &root;
	if( root.get_child_optional("animation") )
	{
		real_root = &root.get_child("animation");
	}

	for(ptree::value_type& child: *real_root)
	{
		if(child.first == "source")
		{
			sources.push_back( load_child<dae_source>(child.second) );
		}
		else if(child.first == "sampler")
		{
			samp = load_child<dae_sampler>(child.second);
		}
		else if(child.first == "channel")
		{
			channel = load_child<dae_channel>(child.second);
		}
	}
	return true;
}


bool dae_sampler::parse(ptree& root)
{
	for(ptree::value_type& child: root)
	{
		if(child.first == "input")
		{
			dae_input_ptr input = load_child<dae_input>(child.second);
			if(input->semantic) {
				if (*input->semantic == "INPUT")
				{
					data_in = input;
				}
				else if (*input->semantic == "OUTPUT")
				{
					data_out = input;
				}
				else if(*input->semantic == "INTERPOLATION")
				{
					interpolation = input;
				}
				else
				{
					assert(false);
				}
			}
		}
	}

	return true;
}

bool dae_channel::parse(ptree& root)
{
	target = root.get_optional<string>("<xmlattr>.target");
	return true;
}

bool dae_matrix::parse(ptree& root)
{
	parse_array( mat.begin(), mat.end(), root.get_value<string>() );
	return true;
}


dae_node_ptr dae_dom::node_by_path(string const& path)
{
	vector<string> segments;
	boost::algorithm::split( segments, path, boost::algorithm::is_any_of("/") );
	assert(!segments.empty());

	assert(id_nodes.count(segments[0]) > 0);

	dae_node_ptr ret_node = id_nodes[segments[0]];
	for(size_t i_generation = 1; i_generation < segments.size(); ++i_generation)
	{
		unordered_map<std::string, dae_node_ptr>::iterator it
			= ret_node->sid_children.find(segments[i_generation]);
		if( it == ret_node->sid_children.end() )
		{
			return dae_node_ptr();
		}
		ret_node = it->second;
	}

	return ret_node;
}

END_NS_SALVIAX_RESOURCE();