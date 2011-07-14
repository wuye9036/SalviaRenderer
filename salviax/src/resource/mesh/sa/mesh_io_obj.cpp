#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>

#include <salviax/include/resource/texture/freeimage/tex_io_freeimage.h>

#include <salviar/include/buffer.h>
#include <salviar/include/input_layout.h>
#include <salviar/include/renderer.h>

#include <eflib/include/math/math.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>

using boost::unordered_map;
using boost::filesystem3::path;
using boost::filesystem3::absolute;

using std::ifstream;
using std::string;
using std::vector;

using eflib::vec4;

using namespace salviar;

BEGIN_NS_SALVIAX_RESOURCE();

struct obj_mesh_vertex{
	vec4 pos;
	vec4 uv;
	vec4 normal;
};

obj_material::obj_material()
	: name("default")
	, ambient( 0.2f, 0.2f, 0.2f, 1.0f )
	, diffuse( 0.5f, 0.5f, 0.5f, 1.0f )
	, specular( 0.7f, 0.7f, 0.7f, 1.0f )
	, shininess(2), alpha(1.0f)
	, is_specular(true)
	, tex_name("")
{
}

bool load_material( renderer* r, vector<obj_material>& mtls, string const& mtl_file, path const& base_path ){

	std::string mtl_fullpath = ( base_path / mtl_file ).string();

	ifstream mtlf( mtl_fullpath );
	if( !mtlf ) return false;

	std::string cmd;

	obj_material* pmtl = NULL;
	for(;;){
		mtlf >> cmd;
		if( !mtlf ){ break; }
		
		if( cmd == "#" ){
			mtlf.ignore(1000, '\n');
			continue;
		}

		if( cmd == "newmtl" ){
			string mtl_name;
			mtlf >> mtl_name;

			for( size_t i_mtl = 0; i_mtl < mtls.size(); ++i_mtl ){
				if( mtls[i_mtl].name == mtl_name ){
					pmtl = &( mtls[i_mtl] );
					break;
				}
			}
		}

		if( !pmtl ){ continue; }

		if( cmd == "Ka" ) {
			float r, g, b;
			mtlf >> r >> g >> b;
			pmtl->ambient = vec4( r, g, b, 0.0f );
		} else if ( cmd == "Kd" ){
			float r, g, b;
			mtlf >> r >> g >> b;
			pmtl->diffuse = vec4( r, g, b, 0.0f );
		} else if ( cmd == "Ks" ){
			float r, g, b;
			mtlf >> r >> g >> b;
			pmtl->specular = vec4( r, g, b, 0.0f );
		} else if ( cmd == "d" || cmd == "Tr" ){
			mtlf >> pmtl->alpha;
		} else if ( cmd == "Ns" ){
			mtlf >> pmtl->shininess;
		} else if ( cmd == "illum" ){
			int illumination;
			mtlf >> illumination;
			pmtl->is_specular = ( illumination == 2 );
		} else if ( cmd == "map_Kd" ){
			mtlf >> pmtl->tex_name;
			std::_tstring tex_fullpath = eflib::to_tstring( ( base_path / pmtl->tex_name ).string() );
			pmtl->tex = texture_io_fi::instance().load(
				r, tex_fullpath,
				salviar::pixel_format_color_rgba8
				);
		} else {
			// Unrecognized command
		}
		mtlf.ignore(1000, '\n');
	}

	mtlf.close();
	return true;
}

bool load_obj_mesh(
	renderer* r,
	string const& fname,
	vector<obj_mesh_vertex>& verts, vector<uint16_t>& indices,
	vector<uint16_t>& attrs, vector<obj_material>& mtls
	)
{
	ifstream objf(fname);

	if( !objf ) return false;

	std::string obj_cmd;

	vector<vec4> positions;
	vector<vec4> uvs;
	vector<vec4> normals;

	obj_material mtl;
	string mtl_fname;

	// init_material
	mtls.push_back(mtl);

	uint16_t subset = 0;

	unordered_map< uint16_t, uint16_t > position_index_to_vertex_index;
	for(;;){
		objf >> obj_cmd;
		if( !objf ){ break; }
		if( obj_cmd == "#" ){
			; // Comment
		} else if ( obj_cmd == "v" ){
			float x = 0.0f, y = 0.0f, z = 0.0f;
			objf >> x >> y >> z;
			positions.push_back( vec4(x, y, z, 1.0f) );
		} else if ( obj_cmd == "vt" ){
			float u = 0.0f, v = 0.0f;
			objf >> u >> v;
			uvs.push_back( vec4(u, v, 0.0f, 0.0f) );
		} else if ( obj_cmd == "vn" ){
			float x = 0.0f, y = 0.0f, z = 0.0f;
			objf >> x >> y >> z;
			normals.push_back( vec4(x, y, z, 0.0f) );
		} else if ( obj_cmd == "f" ){

			uint16_t pos_index = 0, texcoord_index = 0, normal_index = 0;
			obj_mesh_vertex vert;

			for( uint32_t face_index = 0; face_index < 3; ++face_index ){
				memset( &vert, 0, sizeof(vert) );

				objf >> pos_index;
				vert.pos = positions[pos_index-1];

				if( objf.peek() == '/' ){
					objf.ignore();

					if( objf.peek() != '/' ){
						objf >> texcoord_index;
						vert.uv = uvs[texcoord_index-1];
					}

					if( '/' == objf.peek() ){
						objf.ignore();
						objf >> normal_index;
						vert.normal = normals[ normal_index - 1 ];
					}
				}

				uint16_t vert_index = 0;
				if( position_index_to_vertex_index.count(pos_index) == 0){
					vert_index = static_cast<uint16_t>( verts.size() );
					position_index_to_vertex_index[pos_index] = vert_index;
					verts.push_back( vert );
				} else {
					vert_index = position_index_to_vertex_index[pos_index];
				}
				indices.push_back( vert_index );
			}

			attrs.push_back( subset );
		} else if( obj_cmd == "mtllib" ){
			objf >> mtl_fname;
		} else if( obj_cmd == "usemtl" ){
			string name;
			objf >> name;
			bool found = false;
			for( size_t mtl_index = 0; mtl_index < mtls.size(); ++mtl_index ){
				obj_material* pmtl = &mtls[mtl_index];
				if( pmtl->name == name ){
					found = true;
					subset = static_cast<uint16_t>( mtl_index );
					break;
				}
			}

			if( !found ){
				subset = static_cast<uint16_t>( mtls.size() );
				mtls.push_back( obj_material() );
				mtls.back().name = name;
			}

		} else {
			; // Unrecognized command
		}

		objf.ignore( 1000, '\n' );
	}

	objf.close();

	path base_path = absolute( path(fname) );
	if( !mtl_fname.empty() ){
		load_material( r, mtls, mtl_fname, base_path.parent_path() );
	}
	return true;
}

void construct_meshes(
	std::vector<h_mesh>& meshes,
	salviar::renderer* render,
	vector<obj_mesh_vertex> const& verts, vector<uint16_t> const& indices,
	vector<uint16_t> const& attrs, vector<obj_material> const& mtls 
	)
{
	h_buffer vert_buf = render->create_buffer( sizeof(obj_mesh_vertex) * verts.size() );
	vert_buf->transfer( 0, &verts[0], sizeof( obj_mesh_vertex ), verts.size() );

	vector<input_element_desc> descs;

	descs.push_back( input_element_desc("POSITION", 0, format_r32g32b32a32_float, 0, 0, input_per_vertex, 0 ) );
	descs.push_back( input_element_desc("TEXCOORD", 0, format_r32g32b32a32_float, 0, sizeof(vec4), input_per_vertex, 0 ) );
	descs.push_back( input_element_desc("NORMAL", 0, format_r32g32b32a32_float, 0, sizeof(vec4) * 2, input_per_vertex, 0 ) );

	for( size_t i_mtl = 0; i_mtl < mtls.size(); ++i_mtl ){
		mesh* pmesh = new mesh( render );

		// Fill data
		pmesh->add_vertex_buffer( 0, vert_buf, sizeof(obj_mesh_vertex), 0 );
		pmesh->set_input_element_descs( descs );
		h_attached_data mtl_data( new obj_material(mtls[i_mtl]) );
		pmesh->set_attached_data( mtl_data );


		vector<uint16_t> mesh_indices;

		// Construct vertex indices
		for( size_t i_indices = 0; i_indices < indices.size(); ++i_indices ){
			if( attrs[ i_indices / 3 ] == i_mtl ){
				mesh_indices.push_back( indices[i_indices] );
			}
		}

		// Set mesh indices.
		if ( !mesh_indices.empty() ){
			h_buffer index_buffer = render->create_buffer( sizeof(uint16_t) * mesh_indices.size() );
			index_buffer->transfer( 0, &mesh_indices[0], sizeof(uint16_t), mesh_indices.size() );
			pmesh->set_index_buffer(index_buffer);
			pmesh->set_index_type(format_r16_uint);
			pmesh->set_primitive_count( mesh_indices.size() / 3 );

			meshes.push_back( h_mesh(pmesh) );
		}
	}
}

vector<h_mesh> create_mesh_from_obj( salviar::renderer* render, std::string const& file_name )
{
	vector<obj_mesh_vertex> verts;
	vector<uint16_t> indices;
	vector<uint16_t> attrs;
	vector<obj_material> mtls;

	vector<h_mesh> meshes;

	if( !load_obj_mesh( render, file_name, verts, indices, attrs, mtls ) ){
		return meshes;
	}

	construct_meshes( meshes, render, verts, indices, attrs, mtls );

	return meshes;
}

END_NS_SALVIAX_RESOURCE();