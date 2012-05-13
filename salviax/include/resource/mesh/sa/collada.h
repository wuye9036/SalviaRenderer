#ifndef SALVIAX_COLLADA_H
#define SALVIAX_COLLADA_H

#include <salviax/include/resource/resource_forward.h>

#include <salviar/include/shader_abi.h>
#include <eflib/include/math/matrix.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>
#include <vector>

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
DECLARE_STRUCT_SHARED_PTR(dae_param);
DECLARE_STRUCT_SHARED_PTR(dae_controller);
DECLARE_STRUCT_SHARED_PTR(dae_skin);
DECLARE_STRUCT_SHARED_PTR(dae_vertex_weights);

struct dae_dom
{
	boost::unordered_map<std::string, dae_node_ptr> id_nodes;

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
	static void parse_attribute( boost::property_tree::ptree& xml_node, dae_node_ptr node, dae_dom_ptr file );

	template <typename T>
	boost::shared_ptr<T> node_by_id( std::string const& name )
	{
		return root->get_node<T>(name);
	}

	std::string unqualified_source_name()
	{
		if(!source) return std::string();
		return ( (*source)[0] == '#' ? (*source).substr(1) : (*source) );
	}

	dae_node_ptr source_node()
	{
		if( source ) { return root->get_node<dae_node>(*source); }
		return dae_node_ptr();
	}

	template <typename T>
	bool is_a() const
	{
		return dynamic_cast<T const*>(this) != NULL;
	}

	template <typename T> T* as() {
		return dynamic_cast<T*>(this);
	}

	template <typename T> T const* as() const {
		return dynamic_cast<T const*>(this);
	}

	dae_dom*				root;
	boost::optional<std::string> id, sid, name, source;
	virtual ~dae_node(){}
};

struct dae_mesh: public dae_node
{
	static dae_mesh_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );

	std::vector<dae_source_ptr>		sources;
	std::vector<dae_verts_ptr>		verts;
	std::vector<dae_triangles_ptr>	triangle_sets;
};

struct dae_triangles: public dae_node
{
	static dae_triangles_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );

	size_t							count;
	std::vector<dae_input_ptr>		inputs;
	std::vector<int32_t>			indexes;
	boost::optional<std::string>	material_name;
};

struct dae_verts: public dae_node
{
	static dae_verts_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );

	std::vector<dae_input_ptr>		inputs;
	size_t							count;
	boost::optional<std::string>	material_name;

};

struct dae_input: public dae_node
{
	static dae_input_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );

	boost::optional<std::string>	semantic;
	size_t							offset;
	size_t							 set;
};

struct dae_source: public dae_node
{
	static dae_source_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );

	dae_array_ptr	arr;
	dae_tech_ptr	tech;
};

struct dae_array: public dae_node
{
	static dae_array_ptr parse( std::string const& name, boost::property_tree::ptree& root, dae_dom_ptr file );

	enum array_types {
		none_array,
		float_array,
		int_array,
		name_array,
		idref_array
	};

	size_t element_size()
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

	array_types						array_type;
	size_t							count;
	boost::optional<std::string>	content;

	// Parsed members
	std::vector<int>			int_arr;
	std::vector<float>			float_arr;
	std::vector<std::string>	name_arr;
};

struct dae_param: public dae_node
{
	static dae_param_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );

	bool index( std::string const& index_seq, size_t& idx );
	int  index( std::string const& index_seq );
	int  index_xyzw_stpq();

	enum special_types
	{
		st_none,
		st_name	
	};

	salviar::language_value_types	vtype;
	special_types					stype;
};

struct dae_accessor: public dae_node
{
	static dae_accessor_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );

	size_t						offset, stride, count;
	dae_array_ptr				source_array;
	std::vector<dae_param_ptr>	params;
};

struct dae_tech: public dae_node
{
	static dae_tech_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );
	dae_accessor_ptr accessor;
};

struct dae_controller: public dae_node
{
	static dae_controller_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );
	dae_skin_ptr skin;
};

struct dae_skin: public dae_node
{
	static dae_skin_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );
	eflib::mat44				bind_shape_mat;
	std::vector<dae_source_ptr> joint_sources;
	std::vector<dae_input_ptr>	joint_formats;
	dae_vertex_weights_ptr		weights;
};

struct dae_vertex_weights: public dae_node
{
	static dae_vertex_weights_ptr parse( boost::property_tree::ptree& root, dae_dom_ptr file );
	size_t						count;
	std::vector<dae_input_ptr>	inputs;
	std::vector<uint32_t>		vcount;
	std::vector<uint32_t>		v;
};

END_NS_SALVIAX_RESOURCE();

#endif