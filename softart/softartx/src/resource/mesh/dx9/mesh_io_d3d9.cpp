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

#include "softartx/include/resource/mesh/dx9/mesh_io_d3d9.h"
#include "softart/include/renderer.h"
#include "softart/include/buffer.h"

using namespace std;
using namespace efl;
using namespace boost;
using namespace softartx::utility;

BEGIN_NS_SOFTARTX_RESOURCE()

h_mesh create_mesh_from_dx9mesh(renderer* psr, LPD3DXMESH pmesh)
{
	vec3* pverts = NULL;
	byte* pidxs = NULL;

	pmesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)(&pverts));
	pmesh->LockIndexBuffer(D3DLOCK_READONLY, (void**)(&pidxs));

	DWORD nverts = pmesh->GetNumVertices();
	DWORD bytes_per_vert = pmesh->GetNumBytesPerVertex();
	DWORD nfaces = pmesh->GetNumFaces();
	DWORD bytes_per_idx = 
		(pmesh->GetOptions() & D3DXMESH_32BIT) == 0 ? 2 : 4;

	mesh* psrmesh= new mesh(psr);
	h_mesh ret(psrmesh);
	
	//一个vertex buffer，一个index buffer
	psrmesh->set_buffer_count(2);

	h_buffer hvertbuf = psrmesh->create_buffer(0, nverts*bytes_per_vert);
	h_buffer hidxbuf = psrmesh->create_buffer(1, nfaces*bytes_per_idx*3);

	psrmesh->set_index_type(bytes_per_idx == 2 ? index_int16 : index_int32);
	psrmesh->set_index_buf_id(1);
	psrmesh->set_vertex_buf_id(0);
	psrmesh->set_primitive_count(nfaces);

	//parse dx9 decl to layout
	D3DVERTEXELEMENT9 dxdecl[MAX_FVF_DECL_SIZE];
	pmesh->GetDeclaration(dxdecl);
	input_layout_decl layout;

	//累加流标号。Postion、Normal、TextureCoord，目前仅支持这三种情况
	vector<size_t> positions;
	vector<size_t> normals;
	vector<size_t> texcoords;

	//计算layout
	for(size_t idecl = 0; idecl < MAX_FVF_DECL_SIZE; ++idecl){
		const D3DVERTEXELEMENT9& decl = dxdecl[idecl];
		if(decl.Stream == 0xff) break;
		switch(decl.Usage){
			case(D3DDECLUSAGE_POSITION):
				positions.push_back(idecl);
				break;
			case(D3DDECLUSAGE_NORMAL):
				normals.push_back(idecl);
				break;
			case(D3DDECLUSAGE_TEXCOORD):
				texcoords.push_back(idecl);
				break;
			default:
				break;
		}
	}

	size_t usage_idx_counter = 0;
	for(size_t i = 0; i < positions.size(); ++i)
	{
		layout.push_back( 
			input_element_decl(stream_0, dxdecl[positions[i]].Offset, bytes_per_vert, input_float3, input_register_usage_position, input_register_index(usage_idx_counter)));
		++usage_idx_counter;
	}

	for(size_t i = 0; i < normals.size(); ++i){
		layout.push_back(
			input_element_decl(stream_0, dxdecl[normals[i]].Offset, bytes_per_vert, input_float3, input_register_usage_attribute, input_register_index(usage_idx_counter)));
		++usage_idx_counter;
	}

	for(size_t i = 0; i < texcoords.size(); ++i){
		layout.push_back(
			input_element_decl(stream_0, dxdecl[texcoords[i]].Offset, bytes_per_vert, input_float3, input_register_usage_attribute, input_register_index(usage_idx_counter)));
		++usage_idx_counter;
	}

	psrmesh->set_default_layout(layout);

	//copy index buffer and vertex buffer
	void* pvertdata = hvertbuf->raw_data(0);
	void* pxvertdata;
	pmesh->LockVertexBuffer(D3DLOCK_READONLY, &pxvertdata);
	memcpy(pvertdata, pxvertdata, bytes_per_vert * nverts);
	pmesh->UnlockVertexBuffer();

	void* pidxdata = hidxbuf->raw_data(0);
	void* pxidxdata;
	pmesh->LockIndexBuffer(D3DLOCK_READONLY, &pxidxdata);
	memcpy(pidxdata, pxidxdata, bytes_per_idx*nfaces*3);
	pmesh->UnlockIndexBuffer();

	return ret;
}

h_mesh create_mesh_from_xfile(renderer* psr, d3d9_device* dev, const _tstring& filename)
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

END_NS_SOFTARTX_RESOURCE()
