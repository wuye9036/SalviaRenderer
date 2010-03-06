
#ifndef SASL_SEMANTICS_H
#define SASL_SEMANTICS_H

#include "../enums/enum_base.h" 

struct semantics :
	public enum_base< semantics, int >
	, bitwise_op< semantics >, equal_op< semantics >
{
	friend struct enum_hasher;
private:
	semantics( const storage_type& val ): base_type( val ){}
	
public:
	semantics( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type SV_Target;
	const static this_type BLENDWEIGHT;
	const static this_type NORMAL;
	const static this_type SV_PrimitiveID;
	const static this_type SV_Depth;
	const static this_type POSITIONT;
	const static this_type SV_Position;
	const static this_type DEPTH;
	const static this_type PSIZE;
	const static this_type BINORMAL;
	const static this_type SV_ViewPortArrayIndex;
	const static this_type SV_CullDistance;
	const static this_type VFACE;
	const static this_type SV_RenderTargetArrayIndex;
	const static this_type TEXCOORD;
	const static this_type BLENDINDICES;
	const static this_type SV_VertexID;
	const static this_type POSITION;
	const static this_type VPOS;
	const static this_type SV_SampleIndex;
	const static this_type SV_IsFrontFace;
	const static this_type TESSFACTOR;
	const static this_type SV_ClipDistance;
	const static this_type SV_Coverage;
	const static this_type SV_InstanceID;
	const static this_type TENGENT;


};
#endif
