#include <salviax/include/resource/mesh/sa/mesh_io_obj.h>

#include <eflib/include/math/math.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <fstream>

using boost::unordered_map;

using std::ifstream;
using std::string;
using std::vector;

using eflib::vec4;

BEGIN_NS_SALVIAX_RESOURCE();

struct obj_mesh_vertex{
	vec4 pos;
	vec4 uv;
	vec4 normal;
};

bool load_material( vector<obj_material>& mtls, string const& mtl_file ){
	ifstream mtlf( mtl_file );
	if( !mtlf ) return false;

	std::string cmd;

	obj_material* pmtl = NULL;
	for(;;){
		mtlf >> cmd;
		if( !mtlf ){ break; }
		
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

		if( cmd == "#" ){
			/* Comment */ 
		} else if( cmd == "Ka" ) {
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
		} else {
			// Unrecognized command
		}
		mtlf.ignore(1000, '\n');
	}

	mtlf.close();
	return true;
}

bool load_obj_mesh(
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

	if( !mtl_fname.empty() ){
		return load_material( mtls, mtl_fname );
	}
	return true;
}

void construct_meshes(
	std::vector<h_mesh>& meshes, 
	vector<obj_mesh_vertex> const& verts, vector<uint16_t> const& indices,
	vector<uint16_t> const& attrs, vector<obj_material> const& mtls 
	)
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

vector<h_mesh> create_mesh_from_obj( std::string const& file_name )
{
	vector<obj_mesh_vertex> verts;
	vector<uint16_t> indices;
	vector<uint16_t> attrs;
	vector<obj_material> mtls;

	vector<h_mesh> meshes;

	if( !load_obj_mesh( file_name, verts, indices, attrs, mtls ) ){
		return meshes;
	}

	construct_meshes( meshes, verts, indices, attrs, mtls );

	return meshes;
}

END_NS_SALVIAX_RESOURCE();