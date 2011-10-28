#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/enums/builtin_types.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/for.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/metaprog/util.h>
#include <eflib/include/metaprog/enable_if.h>
#include <eflib/include/diagnostics/assert.h>

#include <boost/type_traits.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

namespace llvm{
	class Argument;
	class Function;
	class Type;
	class Value;
	class LLVMContext;
	class Module;
	class BasicBlock;
	class ConstantInt;
	class ConstantVector;

	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
	> class IRBuilder;
	class ConstantFolder;

	typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
		DefaultIRBuilder;
}

namespace sasl{
	namespace syntax_tree{
		struct node;
		struct tynode;
		struct function_type;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_sctxt;

enum abis{
	abi_c,
	abi_llvm,
	abi_unknown
};

class value_tyinfo{
public:
	friend class cg_service;

	enum classifications{
		unknown_type,
		builtin,
		aggregated
	};

	value_tyinfo(
		sasl::syntax_tree::tynode* sty,
		llvm::Type const* cty,
		llvm::Type const* llty
		);

	value_tyinfo( value_tyinfo const& );
	value_tyinfo& operator = ( value_tyinfo const& );

	sasl::syntax_tree::tynode* typtr() const;
	boost::shared_ptr<sasl::syntax_tree::tynode> tysp() const;
	builtin_types hint() const;

	llvm::Type const* llvm_ty( abis abi ) const;

protected:
	value_tyinfo();

	llvm::Type const*			llvm_tys[2];
	sasl::syntax_tree::tynode*	sty;
	classifications				cls;
};

class value_t{
public:
	friend class cg_service;

	value_t();
	value_t( value_t const& );
	value_t& operator = ( value_t const& );

	enum kinds{
		kind_unknown = 0,
		kind_tyinfo_only = 1,
		kind_swizzle = 2,

		kind_value = 4,
		/// \brief Does treat type as reference if ABI is C compatible.
		///  
		/// An important fact is LLVM's ABI is not same as C API.
		/// If structure was passed into function by value,
		/// C compiler will copy a temporary instance and pass in its pointer on x64 calling convention.
		/// But LLVM will push the instance to stack.
		/// So this varaible will qualify the type of arguments/parameters indicates the compiler.
		/// For e.g. we have a prototype:
		///		void foo( struct S );
		/// If is only called by LLVM code, the IR signature will be 
		///		def foo( %S %arg );
		/// But if it maybe called by external function as convention as "C" code,
		/// The IR signature will be generated as following:
		///		def foo( %S* %arg );
		/// And 'kind' the parameter/argument 'arg' is set to 'kind_ref'.
		kind_ref = 8
	};

	enum accessibilities{
		loadable,
		load_store
	} accesibility;

	/// @name State queriers 
	/// @{

	/// Get service.
	cg_service* service() const;

	/// Return internal llvm value.
	llvm::Value* raw() const;

	/// Load llvm value from value_t.
	llvm::Value* load() const;
	llvm::Value* load( abis abi ) const;
	llvm::Value* load_ref() const;

	void store( value_t const& ) const;

	/// Store llvm value to value_t
	// llvm::Value* store( value_t const& );
	void emplace( value_t const& );
	void emplace( llvm::Value* v, kinds k, abis abi );
	void set_parent( value_t const& v );
	void set_parent( value_t const* v );

	bool storable() const;
	bool load_only() const;

	value_t as_ref() const;
	/// Get type information of value.
	value_tyinfo* get_tyinfo() const;
	/// Get type hint. if type is not built-in type it returns builtin_type::none.
	builtin_types get_hint() const;
	/// Get kind.
	kinds get_kind() const;
	/// Get parent. If value is not a member of aggragation, it return NULL.
	value_t* get_parent() const;
	/// Get ABI.
	abis get_abi() const;
	/// Set Index. It is only make sense if parent is available.
	void set_index( size_t index );
	/// @}

	/// @name Operators
	/// @{
	value_t swizzle( size_t swz_code ) const;
	value_t to_rvalue() const;
	/// @}
protected:
	/// @name Constructor, Destructor, Copy constructor and assignment operator
	/// @{
	value_t(
		value_tyinfo* tyinfo,
		llvm::Value* val, value_t::kinds k, abis abi,
		cg_service* cg
		);
	value_t(
		builtin_types hint,
		llvm::Value* val, value_t::kinds k, abis abi,
		cg_service* cg
		);

	static value_t slice( value_t const& vec, uint32_t masks );
	/// @}

	/// @name Members
	/// @{
	boost::scoped_ptr<value_t>	parent; // For write mask and swizzle.
	uint32_t					masks;
	
	kinds			kind;
	llvm::Value*	val;

	/// Type information
	value_tyinfo*	tyinfo;
	builtin_types	hint;

	/// ABI
	abis			abi;

	cg_service*		cg;
	/// @}
};

template <typename RVT>
struct scope_guard{
	typedef boost::function<RVT ()> on_exit_fn;
	scope_guard( on_exit_fn do_exit ): do_exit(do_exit){}
	~scope_guard(){ do_exit(); }
private:
	on_exit_fn do_exit;
};

struct function_t{
	EFLIB_OPERATOR_BOOL( function_t )
	{
		return NULL != fn;
	}

	function_t();

	/// Get argument's value by index.
	value_t arg( size_t index ) const;
	/// Get argument size.
	size_t arg_size() const;
	/// Set argument name.
	void arg_name( size_t index, std::string const& );
	/// Set arguments name. Size of names must be less than argument size.
	void args_name( std::vector<std::string> const& names );
	/// Return true if argument is a reference.
	bool arg_is_ref( size_t index ) const;

	boost::shared_ptr<value_tyinfo> get_return_ty() const;

	std::vector<llvm::Argument*>		argCache;
	sasl::syntax_tree::function_type*	fnty;
	llvm::Function*						fn;
	bool								c_compatible;
	cg_service*							cg;
};

struct insert_point_t{
	EFLIB_OPERATOR_BOOL( insert_point_t )
	{
		return block != NULL;
	}

	insert_point_t();

	llvm::BasicBlock* block;
};

class cg_service{
public:
	/** @name Emit expressions
	Some simple overloadable operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	value_t emit_cond_expr( value_t cond, value_t const& yes, value_t const& no );
	value_t emit_add( value_t const& lhs, value_t const& rhs );
	value_t emit_mul( value_t const& lhs, value_t const& rhs );
	value_t emit_dot( value_t const& lhs, value_t const& rhs );

	value_t emit_add_ss( value_t const& lhs, value_t const& rhs );
	value_t emit_add_vv( value_t const& lhs, value_t const& rhs );

	value_t emit_dot_vv( value_t const& lhs, value_t const& rhs );

	value_t emit_mul_ss( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_sv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_sm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_vv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_vm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mm( value_t const& lhs, value_t const& rhs );

	value_t emit_extract_col( value_t const& lhs, size_t index );

	template <typename IndexT>
	value_t emit_extract_elem( value_t const& vec, IndexT const& idx ){
		if( vec.storable() ){
			return emit_extract_ref( vec, idx );
		} else {
			return emit_extract_val( vec, idx );
		}
	}

	value_t emit_extract_ref( value_t const& lhs, int idx );
	value_t emit_extract_ref( value_t const& lhs, value_t const& idx );
	value_t emit_extract_val( value_t const& lhs, int idx );
	value_t emit_extract_val( value_t const& lhs, value_t const& idx );

	value_t emit_insert_val( value_t const& lhs, value_t const& idx, value_t const& elem_value );
	value_t emit_insert_val( value_t const& lhs, int index, value_t const& elem_value );

	value_t emit_call( function_t const& fn, std::vector<value_t> const& args );
	/** @} */

	/// @name Emit type casts
	/// @{
	/// Cast between integer types.
	value_t cast_ints( value_t const& v, value_tyinfo* dest_tyi );
	/// Cast integer to float.
	value_t cast_i2f( value_t const& v, value_tyinfo* dest_tyi );
	/// Cast float to integer.
	value_t cast_f2i( value_t const& v, value_tyinfo* dest_tyi );
	/// Cast between float types.
	value_t cast_f2f( value_t const& v, value_tyinfo* dest_tyi );
	/// @}

	/// @name Emit Declarations
	/// @{
	function_t begin_fndecl();
	function_t end_fndecl();
	/// @}

	/// @name Context switchs
	/// @{
	void push_fn( function_t const& fn );
	void pop_fn();

	void set_insert_point( insert_point_t const& ip );
	/// @}

	/// @name Context queries
	/// @{
	bool in_function() const;
	function_t& fn();
	/// @}

	/// @name Emit statement
	/// @{
	void emit_return();
	void emit_return( value_t const& );
	/// @}

	/// @name Emit assignment
	/// @{
	void store( value_t& lhs, value_t const& rhs );
	/// @}

	/// @name Emit values
	/// @{
	value_t null_value( value_tyinfo* tyinfo, abis abi );
	value_t null_value( builtin_types bt, abis abi );

	value_t create_value( value_tyinfo* tyinfo, llvm::Value* val, value_t::kinds k, abis abi );
	value_t create_value( builtin_types hint, llvm::Value* val, value_t::kinds k, abis abi );

	template <typename T>
	value_t create_constant_scalar( T const& v, value_tyinfo* tyinfo, EFLIB_ENABLE_IF_COND( boost::is_integral<T> ) ){
		Value* ll_val = ConstantInt::get( IntegerType::get( context(), sizeof(T) * 8 ), uint64_t(v), boost::is_signed<T>::value );
		if( tyinfo ){
			return create_scalar( ll_val, tyinfo );
		} else {
			// Guess tyinfo.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return value_t();
		}
	}

	template <typename T>
	value_t create_constant_scalar( T const& v, value_tyinfo* tyinfo, EFLIB_ENABLE_IF_COND( boost::is_floating_point<T> ) ){
		Value* ll_val = ConstantFP::get( Type::getFloatTy( context() ), v );

		if( tyinfo ){
			return create_scalar( ll_val, tyinfo );
		} else {
			// Guess tyinfo.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return value_t();
		} 
	}
	value_t create_scalar( llvm::Value* val, value_tyinfo* tyinfo );

	template <typename T>
	value_t create_constant_vector( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );

	value_t create_vector( std::vector<value_t> const& scalars, abis abi );

	template <typename T>
	value_t create_constant_matrix( T const* vals, size_t length, abis abi, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}

	/// @name Emit variables
	/// @{
	value_t create_variable( value_tyinfo const*, abis abi, std::string const& name );
	value_t create_variable( builtin_types bt, abis abi, std::string const& name );

	function_t fetch_function( boost::shared_ptr<sasl::syntax_tree::function_type> const& fn_node );
	/// @}

	/// @name Type emitters
	/// @{
	boost::shared_ptr<value_tyinfo> create_tyinfo( boost::shared_ptr<sasl::syntax_tree::tynode> const& tyn );
	/// @}

	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( size_t tyid ) = 0;
	//virtual shared_ptr<sasl::syntax_tree::tynode> get_unique_ty( builtin_types bt ) = 0;

	/// @name Utilities
	/// @{
	insert_point_t new_block( std::string const& hint, bool set_insert_point );
	/// Clean empty blocks of current function.
	virtual void clean_empty_blocks(); 
	virtual cgllvm_sctxt* node_ctxt( boost::shared_ptr<sasl::syntax_tree::node> const& node, bool create_if_need ) = 0;
	/// @}

	/// @name Fundamentals
	/// @{
	virtual llvm::DefaultIRBuilder* builder() const = 0;
	virtual llvm::LLVMContext&		context() const = 0;
	virtual llvm::Module*			module() const = 0;
	/// @}

	/// @name Bridges
	/// @{
	llvm::Type const* type_( builtin_types bt, abis abi );
	template <typename T>
	llvm::ConstantInt* int_(T v);
	template <typename T>
	llvm::ConstantVector* vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	template <typename T>
	llvm::Value* c_vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) );
	/// @}
private:
	std::vector<function_t> fn_ctxts;
};

//template <typename BuilderT>
//class value_t
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
//	value_t<BuilderT>*	parent;
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
//	value_t( llext<BuilderT>* ext ): ext(ext){}
//	value_t( value_t const& ){}
//
//	void clone_to( value_t& lhs );
//	void assign( value_t const& rhs );
//	void proto( value_t& lhs );
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
//	value_t& operator = ( value_t const& );
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