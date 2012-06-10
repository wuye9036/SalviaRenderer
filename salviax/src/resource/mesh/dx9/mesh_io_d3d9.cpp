/*
Copyright (C) 2004-2005 Minmin Gong

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <salviax/include/resource/mesh/dx9/mesh_io_d3d9.h>
#include <salviax/include/resource/mesh/sa/mesh_impl.h>
#include <salviar/include/renderer.h>
#include <salviar/include/buffer.h>

using namespace std;
using namespace eflib;
using namespace boost;
using namespace salviax::utility;
using namespace salviar;

BEGIN_NS_SALVIAX_RESOURCE();

h_mesh create_mesh_from_dx9mesh(salviar::renderer* psr, LPD3DXMESH dx_mesh)
{
	vec3* dx_verts = NULL;
	byte* dx_indices = NULL;

	dx_mesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)(&dx_verts));
	dx_mesh->LockIndexBuffer(D3DLOCK_READONLY, (void**)(&dx_indices));

	DWORD nverts		= dx_mesh->GetNumVertices();
	DWORD vert_size		= dx_mesh->GetNumBytesPerVertex();
	DWORD nfaces		= dx_mesh->GetNumFaces();
	DWORD index_size	= (dx_mesh->GetOptions() & D3DXMESH_32BIT) == 0 ? 2 : 4;

	mesh_impl* psrmesh= new mesh_impl(psr);
	h_mesh ret(psrmesh);
	
	salviar::h_buffer verts = psrmesh->create_buffer( nverts * vert_size );
	salviar::h_buffer indices = psrmesh->create_buffer( nfaces * index_size * 3 );

	//parse dx9 decl to layout
	D3DVERTEXELEMENT9 dx_decls[MAX_FVF_DECL_SIZE];
	dx_mesh->GetDeclaration(dx_decls);

	vector<input_element_desc> elem_descs;
	// Get description
	for(size_t i_decl = 0; i_decl < MAX_FVF_DECL_SIZE; ++i_decl){
		D3DVERTEXELEMENT9* decl = &( dx_decls[i_decl] );

		// The end of declarations
		if(decl->Stream == 0xFF) break;

		const char* semantic_name = NULL;

		switch(decl->Usage){
			case(D3DDECLUSAGE_POSITION):
				semantic_name = "POSITION";
				break;
			case(D3DDECLUSAGE_NORMAL):
				semantic_name = "NORMAL";
				break;
			case(D3DDECLUSAGE_TEXCOORD):
				semantic_name = "TEXCOORD";
				break;
			default:
				semantic_name = NULL;
		}

		if( semantic_name ){
			elem_descs.push_back(
				input_element_desc(
					semantic_name, decl->UsageIndex, format_r32g32b32_float, 0, decl->Offset, input_per_vertex, 0
					)
				);
		}
	}

	// Copy index buffer and vertex buffer
	void* dx_verts_data = NULL;
	dx_mesh->LockVertexBuffer(D3DLOCK_READONLY, &dx_verts_data);
	verts->transfer( 0, dx_verts_data, 0, 0, vert_size * nverts, 1 );
	dx_mesh->UnlockVertexBuffer();

	void* dx_indices_data;
	dx_mesh->LockIndexBuffer(D3DLOCK_READONLY, &dx_indices_data);
	indices->transfer( 0, dx_indices_data, 0, 0, index_size * nfaces * 3, 1 );
	dx_mesh->UnlockIndexBuffer();

	// Set buffer and parameter to mesh.
	psrmesh->set_index_type(index_size == 2 ? format_r16_uint : format_r32_uint);
	psrmesh->set_index_buffer( indices );
	psrmesh->add_vertex_buffer( 0, verts, vert_size, 0 );

	psrmesh->set_input_element_descs( elem_descs );

	psrmesh->set_primitive_count(nfaces);

	return ret;
}

h_mesh create_mesh_from_xfile(salviar::renderer* psr, d3d9_device* dev, const _tstring& filename)
{
	IDirect3DDevice9* d3ddev = dev->get_d3d_device9();
	if(d3ddev == NULL) return h_mesh();

	DWORD material_counts;
	LPD3DXMESH pmesh;
	LPD3DXBUFFER padjbuf;
	LPD3DXBUFFER pmaterials;
	LPD3DXBUFFER peffectinstances;
	D3DXLoadMeshFromX(filename.c_str(), D3DXMESH_SYSTEMMEM, d3ddev, &padjbuf, &pmaterials, &peffectinstances, &material_counts, &pmesh);

	h_mesh ret = create_mesh_from_dx9mesh(psr, pmesh);

	pmesh->Release();
	padjbuf->Release();
	pmaterials->Release();
	peffectinstances->Release();

	return ret;
}

END_NS_SALVIAX_RESOURCE();
