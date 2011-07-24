#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_LLEXT_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_LLEXT_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/enums/builtin_types.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/for.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace llvm{
	class Type;
	class Value;
	class LLVMContext;

	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
	> class IRBuilder;
	class ConstantFolder;

	typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
		DefaultIRBuilder;
}

namespace sasl{
	namespace syntax_tree{
		struct tynode;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class rvalue;

class value_tyinfo{
public:
	friend class cg_service;
	
	enum abis{
		abi_c,
		abi_llvm,
		abi_unknown
	};
	
	enum types{
		builtin,
		aggregated
	};

	value_tyinfo(
		sasl::syntax_tree::tynode* sty,
		llvm::Type const* cty,
		llvm::Type const* llty,
		abis abi
	);
	value_tyinfo( value_tyinfo const& );
	value_tyinfo& operator = ( value_tyinfo const& );

	builtin_types get_hint() const;
	abis get_abi() const;
protected:
	sasl::syntax_tree::tynode*	sty;
	llvm::Type const*			llvm_tys[2];
	abis						abi;
	types						ty;
	builtin_types				hint;
};

class value_proxy{
public:
	friend class code_gen;

	enum kinds{
		kind_tyinfo_only,
		kind_local,
		kind_global,
		kind_member,
		kind_swizzle
	};
	/// Get service.
	cg_service* service() const;

	/// Get LLVM Value of built-in type.
	llvm::Value* get_value() const;

	/// Get type information of value.
	value_tyinfo* get_tyinfo() const;
	builtin_types hint() const;

	kinds get_kind() const;

	rvalue cast_to_rvalue() const;

protected:
	value_proxy();
	value_proxy(
		value_tyinfo* tyinfo,
		llvm::Value* val,
		cg_service* cg
		);

	llvm::Value*		val;
	value_tyinfo*		tyinfo;
	cg_service*			cg;
};

class rvalue : public value_proxy{
public:
	friend class cg_service;

	rvalue( rvalue const& );

	/// Get aggregated value's addr.
	llvm::Value* get_addr() const;

protected:
	rvalue( value_tyinfo* tyinfo, llvm::Value* val, cg_service* cg );
};

class lvalue : public value_proxy{
	rvalue load();
	void store( rvalue const& );
	
	static lvalue allocate();
};

class cgv_member: public lvalue{

};

class cgv_scalar: public rvalue{
public:
	static cgv_scalar from_rvalue( rvalue const& );
	friend cgv_scalar operator + ( cgv_scalar const&, cgv_scalar const& );

	cgv_scalar( cgv_scalar const& );
	cgv_scalar& operator = ( cgv_scalar const& );

protected:
	cgv_scalar();
};

cgv_scalar operator + ( cgv_scalar const&, cgv_scalar const& );

class cgv_vector: public rvalue{
public:
	static cgv_vector from_rvalue( rvalue const& );
	friend cgv_vector operator + ( cgv_vector const& lhs, cgv_vector const& );

	cgv_vector( cgv_vector const& );

	cgv_vector swizzle( size_t swz_code ) const;
private:
	cgv_vector();
	cgv_vector& operator = ( cgv_vector const& );
};

class cgv_matrix: public rvalue{
public:
	cgv_matrix( cgv_matrix const& );
private:
	cgv_matrix();
	cgv_matrix& operator = ( cgv_matrix const& );
};

class cgv_aggragated: public rvalue{
public:
	rvalue operator [] ( size_t sz );
	cgv_aggragated( cgv_aggragated const& );
private:
	cgv_aggragated();
	cgv_aggragated& operator = ( cgv_aggragated const& );
};

class cg_service{
public:
	/** Emit expressions.
	Some simple overloadable operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{  */
	lvalue emit_cond_expr( rvalue cond, lvalue const& yes, lvalue const& no );
	/** @} */
	
	/** Emit type casts @{ */
	/// Cast between integer types.
	rvalue cast_ints( rvalue const& v, value_tyinfo* dest_tyi );
	/// Cast integer to float.
	rvalue cast_i2f( rvalue const& v, value_tyinfo* dest_tyi );
	/// Cast float to integer.
	rvalue cast_f2i( rvalue const& v, value_tyinfo* dest_tyi );
	/// Cast between float types.
	rvalue cast_f2f( rvalue const& v, value_tyinfo* dest_tyi );
	/** @} */
	
	/** Emit outline @{ */
	
	/** @} */
	
	/** Emit statements @{  */
	
	/** @} */
	
	/// Emit assignment @{
	void store( value_proxy& lhs, value_proxy const& rhs );
	/// @}

	/** Emit values @{  */
	template <typename T>
	cgv_scalar create_constant_scalar( T const& v );

	cgv_scalar create_scalar( llvm::Value* val, value_tyinfo* tyinfo );

	template <typename T>
	rvalue create_constant_vector( T const* vals, size_t length, value_tyinfo::abis abi );

	cgv_vector create_vector( std::vector<cgv_scalar> const& scalars, value_tyinfo::abis abi );

	template <typename T>
	rvalue create_constant_matrix( T const* vals, size_t length, value_tyinfo::abis abi );
	/** @} */
	
	/** Emit variables @{ */
	lvalue create_variable( value_tyinfo const* );
	lvalue create_member( value_tyinfo const*, value_proxy::kinds, size_t idx_or_swz );
	/** @} */

	llvm::DefaultIRBuilder* builder() const;
	llvm::LLVMContext&		context() const;
private:
};

//template <typename BuilderT>
//class value_proxy
//{
//public:
//	enum classes{
//		cls_unknown,
//
//		cls_member,
//		cls_swizzle,
//		cls_reference,
//		cls_value
//	};
//
//	enum format{
//		format_abi = 0,
//		format_internal = 1
//	};
//
//private:
//	classes	cls;
//	format	fmt;
//
//	llvm::Value*			val;
//	value_proxy<BuilderT>*	parent;
//	unsigned int			index;
//	llvm::Type const*		vtys[2];
//
//	builtin_types ty_hint;
//
//	llext<BuilderT>* ext; 
//
//	llvm::Value* internal_load() const{
//		if( cls == cls_unknown ){
//			return NULL;
//		}
//		if( cls == cls_value ){
//			return val;
//		}
//		if( cls == cls_reference ){
//			return ext->CreateLoad( val );
//		}
//		if( cls == cls_member ){
//			EFLIB_ASSERT_UNIMPLEMENTED();
//			return NULL;
//		}
//	}
//
//	void internal_store( llvm::Value* v ){
//		assert( cls != cls_unknown );
//
//		if( cls == cls_value ){
//			assert( val );
//			val = v;
//		} else if( cls == cls_reference ){
//			ext->builder->CreateStore( v, val );
//		} else if( cls == cls_member ){
//			assert( parent );
//			llvm::Value* addr = internal_address();
//			if( addr ){
//				ext->builder->CreateStore( v, addr );
//			} else {
//				llvm::Value* parent_val = parent->load();
//				parent_val = ext->builder->CreateInsertValue( parent_val, v, index );
//				parent->store( parent_val );
//			}
//		} else if ( cls == cls_swizzle ){
//			assert( parent );
//			llvm::Value* parent_val = parent->load();
//			parent_val = ext->builder->CreateInsertElement( parent_val, v, index );
//			parent->store( parent_val );
//		}
//	}
//
//	llvm::Value* internal_address(){
//		if( cls == cls_value ){
//			return NULL;
//		}
//		if( cls == cls_reference ){
//			return val;
//		}
//		if( cls == cls_swizzle ){
//			return NULL;
//		}
//		if( cls == cls_member ){
//			Value* indexes[2];
//			indexes[0] = llv_int<BuilderT, 32, true>(0).val;
//			indexes[1] = llv_int<BuilderT, 32, true>(index).val;
//			return ext->builder->GetElementPtr( parent->address(), indexes, indexes+2 );
//		}
//	}
//
//	bool compatible( llvm::Type const* lhs, llvm::Type const* rhs  ){
//		StructType const* struct_ty = cast<StructType>( lhs->isStructTy() ? lhs : rhs );
//		VectorType const* vector_ty = cast<VectorType>( lhs->isVectorTy() ? lhs : rhs );
//
//		if( !(struct_ty && vector_ty) ){ return false;}
//		if( struct_ty->getNumElements() != vector_ty->getNumElements() ){ return false; }
//		llvm::Type const* elem_ty = vector_ty->getElementType();
//		for( size_t i = 0; i < struct_ty->getNumElements(); ++i ){
//			if( struct_ty->getElementType(i) != elem_ty ){
//				return false;
//			}
//		}
//
//		return true;
//	}
//public:
//	value_proxy( llext<BuilderT>* ext ): ext(ext){}
//	value_proxy( value_proxy const& ){}
//
//	void clone_to( value_proxy& lhs );
//	void assign( value_proxy const& rhs );
//	void proto( value_proxy& lhs );
//	
//	llvm::Type const* value_ty() const{
//		return vty[fmt];
//	}
//
//	llvm::Type const* value_ty( format f ) const{
//		return vty[f];
//	}
//
//	llvm::Value* load( format f ) const{
//		if( f == fmt ){
//			return internal_load();
//		} else {
//			return convert(f, fmt, internal_load() );
//		}
//	}
//
//	void store( llvm::Value* v, format f ){
//		if( f == fmt ){
//			internal_store( v );
//		} else {
//			internal_store( convert(fmt, f, v) );
//		}
//	}
//
//	void store( llvm::Value* v ){
//		// Must be basic type.
//		assert( ty_hint != builtin_types::none );
//		format guess_fmt = guess_format(v);
//		store( v, guess_fmt );
//	}
//	
//	format guess_format( llvm::Value* v ){
//		if( v->getType() == vty[format_internal] ){
//			return format_internal;
//		}
//		if( v->getType() == vty[format_abi] ){
//			return format_abi;
//		}
//		assert(false);
//		return format_internal;
//	}
//
//
//
//	llvm::Value const* convert(
//		format dest_fmt, format src_fmt,
//		llvm::Value* source_value
//		)
//	{
//		if( dest_fmt == src_fmt ){
//			return source_value;
//		}
//
//		if( !( is_vector(ty_hint) || is_matrix(ty_hint) ) ){
//			return val;
//		}
//
//		if( is_vector(ty_hint) ){
//			if( source_value->getType()->isVectorTy() ){
//				return llaggregated<BuilderT>( llvector< llvalue<BuilderT> >( v, ext ) ).val;
//			} else {
//				return llvector< llvalue<BuilderT> >( llaggregated<BuilderT>( v, ext ) ).val;
//			}
//		}
//
//		if( is_matrix(ty_hint) ){
//			llaggregated<BuilderT> mat = llaggregated<BuilderT>( source_value, ext );
//			llaggregated<BuilderT> ret = ext->null_value< llaggregated<BuilderT> >( vty[dest_fmt] );
//			for( size_t i = 0; i < mat.size(); ++i){
//				builtin_types row_ty = vector_of( scalar_of(ty_hint), vector_size(ty_hint) );
//				llvm::Value* cvt = convert( row_ty, dest_fmt, src_fmt, mat[i].val );
//				ret.set( i, cvt );
//			}
//			return ret.val;
//		}
//
//		return NULL;
//	}
//
//private:
//	value_proxy& operator = ( value_proxy const& );
//};

//template<typename BuilderT>
//class llvalue{
//public:
//	typedef BuilderT builder_t;
//
//	llvalue( llvm::Value* val, llext<builder_t>* ext )
//		:val(val), ext(ext)
//	{}
//
//	llvalue(): val(NULL), ext(NULL){}
//	llvalue( const llvalue& rhs ): val(rhs.val), ext(rhs.ext){}
//	llvalue& operator = ( llvalue const& rhs ){
//		val = rhs.val;
//		ext = rhs.ext;
//		return *this;
//	}
//	
//	llvm::Value* v() const{
//		return val;
//	}
//	
//public:
//	llvm::Value* val;
//	llext<BuilderT>* ext;
//};
//
//template<typename BuilderT, unsigned int Bits, bool IsSigned>
//class llv_int: public llvalue<BuilderT>{
//public:
//	typedef BuilderT builder_t;
//
//	llv_int( llvm::Value* val, llext<builder_t>* ext ): llvalue( val, ext )
//	{
//	}
//
//	template <typename ValueT>
//	llv_int( llext<builder_t>* ext, ValueT v )
//		:llvalue( ConstantInt::get( IntegerType::get(ext->ctxt, Bits ), static_cast<uint64_t>(v), IsSigned ), ext )
//	{
//	}
//
//	static llv_int<BuilderT, Bits, IsSigned> null_value(){
//		return llv_int<BuilderT, Bits, IsSigned>( ext, 0 );
//	}
//};
//
//template<typename BuilderT>
//class llv_fp: public llvalue<BuilderT>{
//public:
//	typedef BuilderT builder_t;
//
//	llv_fp( llvm::Value* val, llext<builder_t>* ext )
//		:llvalue(val, ext)
//	{}
//
//	llv_fp( llext<builder_t>* ext, float v )
//		:llvalue( ConstantFP::get( Type::getFloatTy(ext->ctxt), v ), ext )
//	{
//	}
//	llv_fp( llext<builder_t>* ext, double v )
//		:llvalue( ConstantFP::get( Type::getDoubleTy(ext->ctxt), v ), ext )
//	{
//	}
//
//	static llv_fp<builder_t> null_value(){
//		return llv_fp<builder_t>( ext, 0.0 );
//	}
//};
//
//template<typename ElementT>
//class llvector: public llvalue< typename ElementT::builder_t >{
//public:
//	typedef typename ElementT::builder_t builder_t;
//
//	typedef llvector<ElementT> this_type;
//
//	llvector( llvm::Value* val, llext<builder_t>* ext )
//		:llvalue(NULL, ext)
//	{
//		if( !val || val->getType()->isVectorTy() ) {
//			this->val = val;
//		} else {
//			this->val = create( ext, val ).val;
//		}
//	}
//
//	llvector( std::vector<ElementT> const& v )
//	{
//		assert( !v.empty() );
//
//		ext = v[0].ext;
//		llvector<ElementT> ret( ext->null_value( v[0] ), v.size() );
//		val = ret.val;
//
//		for( size_t idx = 0; idx < v.size(); ++idx ){
//			set( idx, v[idx] );
//		}
//	}
//
//	llvector( llaggregated<builder_t> const& v ){
//		llvector< llvalue<builder_t> > ret( v[0], v.size() );
//		val = ret.val;
//		ext = v.ext;
//		for( size_t idx = 0; idx < v.size(); ++idx ){
//			set( idx, ElementT( v[idx].val, ext ) );
//		}
//	}
//
//	template <typename ValueT>
//	static llvector<ElementT> from_values( llext<builder_t>* ext, std::vector<ValueT> const& v ){
//		std::vector<ElementT> elems;
//		elems.reserve(v.size());
//		for( size_t i = 0; i < v.size(); ++i ){
//			elems.push_back( ElementT( ext, v[i] ) );
//		}
//		return llvector<ElementT>( elems );
//	}
//
//	template <typename ValueT>
//	static llvector<ElementT> create( llext<builder_t>* ext, ValueT* v ){
//		if( v->getType()->isVectorTy() ){
//			return llvector<ElementT>(v, ext);
//		} else if( v->getType()->isStructTy() ){
//			return llvector<ElementT>( llaggregated<builder_t>(v, ext) );
//		} else {
//			assert(false);
//			return llvector<ElementT>(NULL, ext);
//		}
//	}
//
//	llvector( ElementT const& v, size_t nvec ){
//		ext = v.ext;
//
//		vector<Constant*> vals;
//		for( size_t idx = 0 ; idx < nvec; ++idx ){
//			vals.push_back( cast<Constant>( ext->null_value(v).val) );
//		}
//
//		llvm::VectorType const* vtype = VectorType::get( v.val->getType(), static_cast<unsigned>( nvec ) );
//		val = ConstantVector::get( vtype, vals );
//
//		for( size_t idx = 0; idx < nvec; ++idx ){
//			set( idx, v );
//		}
//	};
//
//	size_t size() const{
//		return val ? cast<llvm::VectorType>(val->getType())->getNumElements() : 0;
//	}
//
//	llvector<ElementT> swizzle( std::vector<int> const& indices ) const{
//		typedef llvector< llv_int<builder_t, 32, true> > i32vec;
//
//		i32vec masks = i32vec::from_values( ext, indices );
//		return llvector<ElementT>(
//			ext->builder->CreateShuffleVector(
//				val, UndefValue::get(val->getType()),
//				masks.val ),
//				ext
//			);
//	}
//	
//	template <typename IndexT>
//	llvector<ElementT> swizzle( IndexT* indices, int max_index = 0 ) const{
//		std::vector<int> index_values;
//		int step = 0;
//		while( indices[step] != -1 && ( max_index == 0 || step < max_index ) ){
//			index_values.push_back( indices[step] );
//		}
//		return swizzle( index_values );
//	}
//
//	llvector<ElementT>& set( size_t idx, ElementT const& v ){
//		val = ext->builder->CreateInsertElement( val, v.v(), llv_int<builder_t, 32, true>(ext, idx).val );
//		return *this;
//	}
//
//	ElementT operator []( size_t idx ) const{
//		return ElementT( ext->builder->CreateExtractElement( val, llv_int<builder_t, 32, true>(ext, idx).val ), ext );
//	}
//};
//
//template <typename ElementT> class llvar;
//
//template <typename ElementT>
//class llptr: public llvalue<typename ElementT::builder_t>{
//public:
//	typedef typename ElementT::builder_t builder_t;
//	
//	llptr( llvm::Value* ptr, llext<builder_t>* ext )
//		: llvalue<builder_t>( ptr, ext )
//	{
//	}
//
//	ElementT operator *(){
//		return (ElementT)( (*this)[0] );
//	}
//
//	llvar<ElementT> operator [] ( size_t index ){
//		llv_int<builder_t, 32, true> index_value(ext, index);
//		llvm::Value* elem_ptr = ext->builder->CreateGEP( val, index_value );
//		return llvar<ElementT>( elem_ptr, ext );
//	}
//
//	llptr<ElementT> operator + ( size_t index ){
//		llv_int<builder_t, 32, true> index_value(ext, index);
//		llvm::Value* elem_ptr = ext->builder->CreateGEP( val, index_value );
//		return llptr<ElementT>( elem_ptr, ext );
//	}
//};
//
//template<typename ElementT>
//class llvar: llvalue< typename ElementT::builder_t >{
//public:
//	typedef typename ElementT::builder_t builder_t;
//	
//	llvar( llvm::Type const* var_type, llext<builder_t>* ext ){
//		val = ext->builder->CreateAlloca( var_type );
//		this->ext = ext;
//	}
//
//	llvar( llvm::Value* val, llext<builder_t>* ext ){
//		val = ext->builder->CreateAlloca( val->getType() );
//		this->ext = ext;
//	}
//
//	llptr<ElementT> operator &(){
//		return llptr<ElementT>( val, ext );
//	}
//
//	operator ElementT(){
//		LoadInst* inst = ext->builder->CreateLoad(val);
//		return ElementT( inst, ext );
//	}
//
//	llvar<ElementT> operator = ( ElementT const& v ){
//		llvm::StoreInst* inst = ext->builder->CreateStore( v.val, val );
//	}
//};
//
//template <typename BuilderT>
//class llaggregated: public llvalue<BuilderT>{
//public:
//	typedef BuilderT builder_t;
//	
//	llaggregated( llvm::Value* val, llext<builder_t>* ext )
//		: llvalue( val, ext )
//	{
//		if( !val || val->getType()->isAggregateType() ){
//			this->val = val;
//		} else {
//			this->val = create( ext, val ).val;
//		}
//	}
//
//	template <typename ElementT>
//	llaggregated( llvector<ElementT> const& v ){
//		ext = v.ext;
//		size_t size = v.size();
//
//		llvm::StructType const* struct_ty
//			= StructType::get( ext->ctxt, vector<Type const*>( size, v[0].val->getType() ), true );
//		val = ext->null_value< llvalue<BuilderT> >(struct_ty).val;
//		
//		for( size_t idx = 0; idx < size; ++idx ){
//			set(idx, v[idx]);
//		}
//
//		
//	}
//
//	static llaggregated<BuilderT> create(
//		llext<builder_t>* ext, llvm::Value* val
//		)
//	{
//		if( val->getType()->isVectorTy() ){
//			return llaggregated<BuilderT>( llvector< llvalue<builder_t> >(val, ext) );
//		} else if( val->getType()->isAggregateType() ) {
//			return llaggregated<BuilderT>( val, ext );
//		} else {
//			assert( false );
//			return llaggregated<BuilderT>( NULL, ext );
//		}
//	}
//
//	size_t size() const{
//		if ( !val ) return 0;
//		if( val->getType()->isStructTy() ){
//			return cast<StructType>(val->getType())->getNumElements();
//		} else if(val->getType()->isArrayTy()) {
//			return cast<ArrayType>(val->getType())->getNumElements();
//		} else {
//			return 0;
//		}
//	}
//
//	template <typename ElementT>
//	llaggregated<BuilderT>& set( size_t idx, ElementT const& v ){
//		val = ext->builder->CreateInsertValue( val, v.val, static_cast<unsigned>(idx) );
//		return *this;
//	}
//
//	template <typename ElementT>
//	ElementT get( size_t idx ) const{
//		return ElementT( ext->builder->CreateExtractValue( val, static_cast<unsigned>(idx) ), ext );
//	}
//
//	llvalue<BuilderT> operator [] ( size_t idx ) const{
//		return get< llvalue<BuilderT> >(idx);
//	}
//};
//
//template<typename ElementT>
//class llarray: public llvalue< typename ElementT::builder_t >{
//public:
//	typedef typename ElementT::builder_t builder_t;
//
//	typedef llarray<ElementT> this_type;
//
//	llarray( llvm::Value* val, llext<builder_t>* ext )
//		:llvalue(val, ext)
//	{}
//
//	llarray<ElementT> set( size_t idx, ElementT const& v ){
//		val = ext->builder->CreateInsertValue( val, v.v(), static_cast<unsigned>(idx) );
//	}
//
//	ElementT operator []( size_t idx ){
//		return ElementT( ext->builder->CreateExtractValue( val, static_cast<unsigned>(idx) ), ext );
//	}
//};
//
//
//class ll_block{
//public:
//	ll_block( llvm::BasicBlock* block ): block(block){}
//	ll_block(): block(NULL){}
//
//	operator bool() const{
//		return block != NULL;
//	}
//	llvm::BasicBlock* block;
//};
//
//#define LLEXT_CONTEXT_MEMBERS() \
//	((llvalue<BuilderT>,	cond_val	)) \
//	((ll_block,				cond_block	)) \
//	((ll_block,				then_block	)) \
//	((ll_block,				else_block	))
//
//#define LLEXT_CONTEXT_DECLARE_MEMBER( r, data, elem ) \
//	BOOST_PP_TUPLE_ELEM( 2, 0, elem) BOOST_PP_TUPLE_ELEM( 2, 1, elem ) ;
//
//#define LLEXT_CONTEXT_DECLARE_MEMBERS()	\
//	BOOST_PP_SEQ_FOR_EACH( LLEXT_CONTEXT_DECLARE_MEMBER, _, LLEXT_CONTEXT_MEMBERS() )
//	
//#define LLEXT_CONTEXT_PUSH_MEMBER( r, data, elem )	\
//	ctxts.back().BOOST_PP_TUPLE_ELEM(2, 1, elem) = BOOST_PP_TUPLE_ELEM(2, 1, elem);
//
//#define LLEXT_CONTEXT_PUSH() \
//	ctxts.push_back( llext_context<BuilderT>() );	\
//	BOOST_PP_SEQ_FOR_EACH( LLEXT_CONTEXT_DECLARE_MEMBER, _, LLEXT_CONTEXT_MEMBERS() );
//	
//#define LLEXT_CONTEXT_POP_MEMBER( r, data, elem )	\
//	BOOST_PP_TUPLE_ELEM(2, 1, elem) = ctxts.back().BOOST_PP_TUPLE_ELEM(2, 1, elem);
//
//#define LLEXT_CONTEXT_POP() \
//	BOOST_PP_SEQ_FOR_EACH( LLEXT_CONTEXT_DECLARE_MEMBER, _, LLEXT_CONTEXT_MEMBERS() ); \
//	ctxts.pop_back();
//
//template <typename BuilderT>
//struct llext_context{
//	LLEXT_CONTEXT_DECLARE_MEMBERS();
//};
//
//template <typename BuilderT>
//class llext{
//public:
//	typedef llvalue<BuilderT> llvalue_t;
//
//	llext( llvm::LLVMContext& ctxt, BuilderT* builder )
//		:ctxt( ctxt ), builder( builder ){}
//
//	llvalue<BuilderT> null_value( llvm::Type const* t ){
//		return llvalue<BuilderT>( Constant::getNullValue(t), this );
//	}
//
//	template<typename T>
//	T null_value( llvm::Type const* t ){
//		return T( Constant::getNullValue(t), this );
//	}
//
//	template <typename ElementT>
//	ElementT null_value( ElementT const& v ){
//		return ElementT( Constant::getNullValue(v.val->getType()), this );
//	}
//
//public:
//	// Fake statements
//
//	/** If ... Then ... Else ... End If statement
//	@{ */
//	void if_( llvalue<BuilderT> const& cond ){
//		assert( cond.val->getType()->isIntegerTy(1) );
//
//		// Save cond, then, else.
//		push();
//
//		cond_block = ll_block( builder->GetInsertBlock() );
//		cond_val = cond;
//		else_block = then_block = ll_block();
//	}
//	void then_(){
//		assert( cond_block );
//		then_block = block( "then" );
//		insert_to( then_block );
//	}
//	void else_(){
//		assert( then_block );
//		then_block = current_block();
//		else_block = block( "else" );
//		insert_to( else_block );
//	}
//	void end_if(){
//		assert( cond_block && then_block );
//
//		ll_block cur_block = block( "endif" );
//
//		if( then_block ){
//			insert_to(then_block);
//			goto_(cur_block);
//		}
//
//		if( else_block ){
//			insert_to(else_block);
//			goto_(cur_block);
//		} else {
//			else_block = cur_block;
//		}
//
//		insert_to( cond_block );
//		goto_( cond_val, then_block, else_block );
//
//		insert_to( cur_block );
//
//		// Recovery cond, then, else.
//		pop();
//	}
//	/** @} */
//
//	/** Jump statement @{ */ 
//	void goto_( llvalue<BuilderT> const& cond_v, ll_block const& yes, ll_block const& no ){
//		builder->CreateBr( cond_v.val, yes.block, no.block );
//	}
//
//	void goto_( ll_block const& block ){
//		builder->CreateBr( block.block );
//	}
//	void return_( llvalue<BuilderT> const& v ){
//		builder->CreateRet(v.val);
//	}
//
//	void return_(){
//		builder->CreateRetVoid();
//	}
//	/** @} */
//
//	llvm::LLVMContext& ctxt;
//	BuilderT* builder;
//
//private:
//
//	LLEXT_CONTEXT_DECLARE_MEMBERS();
//
//	/** Utility functions
//	@{*/
//
//	ll_block current_block(){
//		return ll_block( builder->GetInsertBlock() );
//	}
//
//	void insert_to( ll_block const& block ){
//		builder->SetInsertBlock( block.block );
//	}
//
//	void push_insert_point(){
//		insert_points.push_back(
//			make_pair( builder->GetInsertBlock(), builder->GetInsertPoint() )
//			);
//	}
//
//	void pop_insert_point(){
//		builder->SetInsertPoint( insert_points.back().first, insert_points.back().second );
//		insert_points.pop_back();
//	}
//
//	void push(){
//		LLEXT_CONTEXT_PUSH();
//	}
//
//	void pop(){
//		LLEXT_CONTEXT_POP();
//	}
//
//	/** @} */
//
//	// attributes
//	std::vector< llext_context<BuilderT> > ctxts;
//	std::vector< std::pair<llvm::BasicBlock*, llvm::BasicBlock::iterator> > insert_points;
//
//	llext( llext<BuilderT> const& );
//	llext& operator = ( const llext<BuilderT>& );
//};
//
///* ------------------------------------------------------------------------- */
//
//// Operator +
//template <typename BuilderT>
//llv_fp<BuilderT> operator + ( llv_fp<BuilderT> const& lhs, llv_fp<BuilderT> const& rhs ){
//	llext<BuilderT>* ext = lhs.ext;
//	return llv_fp<BuilderT>( ext->builder->CreateFAdd( lhs.v(), rhs.v() ), ext );
//}
//
//template <typename BuilderT>
//llv_int<BuilderT, 32, true> operator + ( llv_int<BuilderT, 32, true> const& lhs, llv_int<BuilderT, 32, true> const& rhs ){
//	llext<BuilderT>* ext = lhs.ext;
//	return llv_int<BuilderT, 32, true>( ext->builder->CreateAdd( lhs.v(), rhs.v() ), ext );
//}
//
//template <typename BuilderT>
//llvector< llv_fp<BuilderT> > operator + (
//	llvector< llv_fp<BuilderT> > const& lhs,
//	llvector< llv_fp<BuilderT> > const& rhs
//	)
//{
//	llext<BuilderT>* ext = lhs.ext;
//	return llvector< llv_fp<BuilderT> >( ext->builder->CreateFAdd( lhs.v(), rhs.v() ), ext );
//}
//
///* ------------------------------------------------------------------------- */
//
///* ------------------------------------------------------------------------- */
//
//// Operator *
//template<typename BuilderT>
//llv_fp<BuilderT> operator * ( llv_fp<BuilderT> const& lhs, llv_fp<BuilderT> const& rhs ){
//	llext<BuilderT>* ext = lhs.ext;
//	return llv_fp<BuilderT>( ext->builder->CreateFMul( lhs.v(), rhs.v() ), ext );
//}
//
//template<typename BuilderT>
//llv_int<BuilderT, 32, true> operator * ( llv_int<BuilderT, 32, true> const& lhs, llv_int<BuilderT, 32, true> const& rhs ){
//	llext<BuilderT>* ext = lhs.ext;
//	return llv_int<BuilderT, 32, true>( ext->builder->CreateMul( lhs.v(), rhs.v() ), ext );
//}
//
//template <typename BuilderT>
//llvector< llv_fp<BuilderT> > operator * (
//	llvector< llv_fp<BuilderT> > const& lhs,
//	llvector< llv_fp<BuilderT> > const& rhs
//	)
//{
//	llext<BuilderT>* ext = lhs.ext;
//	return llvector< llv_fp<BuilderT> >( ext->builder->CreateFMul( lhs.v(), rhs.v() ), ext );
//}
/* ------------------------------------------------------------------------- */
END_NS_SASL_CODE_GENERATOR();

#endif