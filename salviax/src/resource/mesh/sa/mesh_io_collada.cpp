#include <salviax/include/resource/mesh/sa/mesh_io_collada.h>

#include <salviar/include/renderer.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>
#include <sstream>

using salviar::renderer;
using boost::property_tree::ptree;
using boost::property_tree::read_xml;
using boost::optional;
using boost::make_shared;
using boost::unordered_map;
using std::vector;
using std::string;
using std::fstream; 
using std::stringstream;

BEGIN_NS_SALVIAX_RESOURCE();

#define DECLARE_STRUCT_SHARED_PTR( name ) struct name; typedef boost::shared_ptr<name> name##_ptr;

DECLARE_STRUCT_SHARED_PTR(dae_source);
DECLARE_STRUCT_SHARED_PTR(dae_verts);
DECLARE_STRUCT_SHARED_PTR(dae_array);
DECLARE_STRUCT_SHARED_PTR(dae_tech);
DECLARE_STRUCT_SHARED_PTR(dae_accessor);
DECLARE_STRUCT_SHARED_PTR(dae_triangles);
DECLARE_STRUCT_SHARED_PTR(dae_input);
DECLARE_STRUCT_SHARED_PTR(dae_mesh);
DECLARE_STRUCT_SHARED_PTR(dae_node);
DECLARE_STRUCT_SHARED_PTR(dae_dom);

struct dae_dom
{
	unordered_map<string, dae_node_ptr> id_nodes;
	
	template <typename T>
	boost::shared_ptr<T> get_node( std::string const& name )
	{
		std::string unqual_name = (name[0] == '#' ? name.substr(1) : name);
		unordered_map<string, dae_node_ptr>::iterator it = id_nodes.find(unqual_name);
		if( it == id_nodes.end() ) { return boost::shared_ptr<T>(); }
		return boost::shared_dynamic_cast<T>(it->second);
	}

	template <typename T>
	void get_node( std::string const& name, boost::shared_ptr<T>& ret )
	{
		ret = get_node<T>(name);
		return;
	}

};

struct dae_node
{
	static void parse_attribute( ptree& xml_node, dae_node_ptr node, dae_dom_ptr file )
	{
		node->id		= xml_node.get_optional<string>("<xmlattr>.id");
		node->sid		= xml_node.get_optional<string>("<xmlattr>.sid");
		node->name		= xml_node.get_optional<string>("<xmlattr>.name");
		node->source	= xml_node.get_optional<string>("<xmlattr>.source");

		if( node->id )
		{
			file->id_nodes.insert( make_pair(*node->id, node) );
		}
	}

	optional<string> id, sid, name, source;
	virtual ~dae_node(){}
};

struct dae_mesh: public dae_node
{
	static dae_mesh_ptr parse( ptree& root, dae_dom_ptr file );

	vector<dae_source_ptr>		sources;
	vector<dae_verts_ptr>		verts;
	vector<dae_triangles_ptr>	triangle_sets;
};

struct dae_triangles: public dae_node
{
	static dae_triangles_ptr parse( ptree& root, dae_dom_ptr file )
	{
		dae_triangles_ptr ret = make_shared<dae_triangles>();
		parse_attribute(root, ret, file);

		EFLIB_ASSERT_UNIMPLEMENTED();

		return ret;
	}
	vector<dae_input_ptr>	inputs;
	string					indexes;
};

struct dae_verts: public dae_node
{
	static dae_verts_ptr parse( ptree& root, dae_dom_ptr file );

	vector<dae_input_ptr>	inputs;
	size_t					count;
	dae_source_ptr			data_source;
	optional<string>		material_name;
	
};

struct dae_input: public dae_node
{
	static dae_input_ptr parse( ptree& root, dae_dom_ptr file )
	{
		dae_input_ptr ret = make_shared<dae_input>();
		parse_attribute(root, ret, file);

		ret->semantic	= root.get_optional<string>("<xmlattr>.semantic");
		ret->offset		= root.get("<xmlattr>.offset",	(size_t)0);
		ret->set		= root.get("<xmlattr>.set",		(size_t)0);
		if(ret->source) { ret->data_source = file->get_node<dae_source>(*ret->source); }

		return ret;
	}

	optional<string>	semantic;
	dae_source_ptr		data_source;
	size_t				offset;
	size_t				set;
};

struct dae_source: public dae_node
{
	static dae_source_ptr parse( ptree& root, dae_dom_ptr file );

	vector<dae_array_ptr>	arrays;
	dae_tech_ptr			tech;
};

struct dae_array: public dae_node
{
	static dae_array_ptr parse( std::string const& name, ptree& root, dae_dom_ptr file )
	{
		dae_array_ptr ret = make_shared<dae_array>();
		parse_attribute(root, ret, file);

		if( name == "float_array" ) ret->array_type = float_array;
		if( name == "int_array" )	ret->array_type = int_array;

		ret->count = root.get( "<xmlattr>.count", 0 );
		ret->content = root.get_value_optional<string>();

		// Parse elements
		if( ret->content )
		{
			switch (ret->array_type)
			{
			case float_array:
				{
					stringstream ss( *ret->content );
					float v = 0.0f;
					while( ss >> v ) { ret->float_arr.push_back(v); }
					if( ret->count == 0 ) { ret->count = ret->float_arr.size(); }
				}
				break;
			case int_array:
				{
					stringstream ss( *ret->content );
					int v = 0.0f;
					while( ss >> v ) { ret->int_arr.push_back(v); }
					if( ret->count == 0 ) { ret->count = ret->int_arr.size(); }
				}
				break;
			}
		}

		return ret;
	}

	enum array_types {
		none_array,
		float_array,
		int_array,
		idref_array
	};

	array_types			array_type;
	size_t				count;
	optional<string>	content;

	// Parsed members
	vector<int>		int_arr;
	vector<float>	float_arr;
};

struct dae_accessor: public dae_node
{
	static dae_accessor_ptr parse( ptree& root, dae_dom_ptr file )
	{
		dae_accessor_ptr ret = make_shared<dae_accessor>();
		parse_attribute(root, ret, file);

		ret->source_array = file->get_node<dae_array>(*ret->source);
		
		if( ret->source_array )
		{
			ret->offset = root.get( "<xmlattr>.offset", size_t(0) );
			ret->stride = root.get( "<xmlattr>.stride", size_t(0) );
			ret->count  = root.get( "<xmlattr>.count" , size_t(0) );
		}
		else
		{
			ret.reset();
		}

		return ret;
	}

	size_t			offset, stride, count;
	dae_array_ptr	source_array;
};

struct dae_tech: public dae_node
{
	static dae_tech_ptr parse( ptree& root, dae_dom_ptr file )
	{
		dae_tech_ptr ret = make_shared<dae_tech>();
		parse_attribute(root, ret, file);

		optional<ptree&> accessor_xml_node = root.get_child_optional("accessor");

		if( !accessor_xml_node ) {
			ret.reset(); 
		} else {
			ret->accessor = dae_accessor::parse( *accessor_xml_node, file );
		}
		
		return ret;
	}

	dae_accessor_ptr accessor;
};

dae_mesh_ptr dae_mesh::parse( ptree& root, dae_dom_ptr file )
{
	dae_mesh_ptr ret = make_shared<dae_mesh>();

	parse_attribute(root, ret, file);

	BOOST_FOREACH( ptree::value_type& child, root )
	{
		if( child.first == "<xmlattr>" ) { continue; }

		if( child.first == "source" ){
			ret->sources.push_back( dae_source::parse(child.second, file) );
		} else if( child.first == "vertices" ) {
			ret->verts.push_back( dae_verts::parse(child.second, file) );
		} else if( child.first == "triangles" ) {
			ret->triangle_sets.push_back( dae_triangles::parse(child.second, file) );
		} else {
			assert(false);
		}
	}

	return ret;
}

dae_source_ptr dae_source::parse( ptree& root, dae_dom_ptr file )
{
	dae_source_ptr ret = make_shared<dae_source>();
	parse_attribute(root, ret, file);

	BOOST_FOREACH( ptree::value_type& child, root )
	{
		if( child.first == "<xmlattr>" ) { continue; }

		if(/**/child.first == "float_array"
			|| child.first == "int_array" )
		{
			ret->arrays.push_back( dae_array::parse(child.first, child.second, file) );
		}
		else if( child.first == "technique_common" )
		{
			ret->tech = dae_tech::parse(child.second, file);
		}
		else
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	return ret;
}

dae_verts_ptr dae_verts::parse( ptree& root, dae_dom_ptr file )
{
	dae_verts_ptr ret = make_shared<dae_verts>();
	parse_attribute(root, ret, file);

	BOOST_FOREACH( ptree::value_type& child, root )
	{
		if( child.first == "<xmlattr>" )
		{
			ret->count = child.second.get("count", (size_t)0);
			ret->material_name = child.second.get_optional<string>("material");
		}
		else if( child.first == "input" )
		{
			ret->inputs.push_back( dae_input::parse(child.second, file) );
		}
	}

	return ret;
}

h_mesh build_mesh( dae_mesh_ptr m )
{
	return h_mesh();
}

h_mesh create_mesh_from_collada( renderer* render, std::string const& file_name )
{
	h_mesh ret;
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
					return build_mesh( dae_mesh_node );
				}
			}
		}
	}

	return ret;
}

END_NS_SALVIAX_RESOURCE();

