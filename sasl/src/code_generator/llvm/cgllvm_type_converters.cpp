#include <eflib/include/platform/disable_warnings.h>
#include <LLVM/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>
#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/semantic/type_manager.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/utility.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/diagnostics/assert.h>

#include <vector>

using ::llvm::IRBuilderBase;
using ::llvm::IRBuilder;
using ::llvm::Type;
using ::llvm::Value;
using ::llvm::VectorType;

using ::sasl::semantic::encode_swizzle;
using ::sasl::semantic::encode_sized_swizzle;
using ::sasl::semantic::symbol;
using ::sasl::semantic::type_converter;
using ::sasl::semantic::type_entry;
using ::sasl::semantic::type_info_si;
using ::sasl::semantic::type_manager;

using ::sasl::syntax_tree::create_builtin_type;
using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::tynode;

using ::boost::bind;
using ::boost::function;
using ::boost::make_shared;
using ::boost::shared_polymorphic_cast;
using ::boost::shared_ptr;
using ::boost::shared_static_cast;

using std::vector;

using namespace sasl::utility;

BEGIN_NS_SASL_CODE_GENERATOR();

class cg_service;

typedef function< cgllvm_sctxt* ( shared_ptr<node> const& ) > get_ctxt_fn;

class cgllvm_type_converter : public type_converter{
public:
	cgllvm_type_converter( get_ctxt_fn get_ctxt, cg_service* cgs )
		: get_ctxt(get_ctxt), cgs(cgs)
	{
	}

	// TODO if dest == src, maybe some bad thing happen ...
	void int2int( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );

		value_proxy casted = cgs->cast_ints(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);

		cgs->store( dest_ctxt->get_value(), casted );
	}

	void int2float( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);
		
		assert( src_ctxt != dest_ctxt );

		value_proxy casted = cgs->cast_i2f(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);
		cgs->store( dest_ctxt->get_value(), casted );
	}

	void float2int( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );

		value_proxy casted = cgs->cast_f2i(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);
		cgs->store( dest_ctxt->get_value(), casted );
	}

	void float2float( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );

		value_proxy casted = cgs->cast_f2f(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);
		cgs->store( dest_ctxt->get_value(), casted );
	}

	void scalar2vec1( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );

		value_proxy scalar_value = src_ctxt->get_rvalue();
		vector<value_proxy> scalars;
		scalars.push_back( scalar_value );

		value_proxy vector_value = cgs->create_vector( scalars, dest_ctxt->get_tyinfo_ptr()->get_abi() );

		cgs->store( dest_ctxt->get_value(), vector_value );
	}

	void shrink_vector( shared_ptr<node> dest, shared_ptr<node> src, int source_size, int dest_size ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );
		assert( source_size > dest_size );

		value_proxy vector_value = dest_ctxt->get_rvalue();
		size_t swz_code = encode_sized_swizzle(dest_size);

		cgs->store(
			dest_ctxt->get_value(),
			vector_value.swizzle(swz_code)
			);
	}
private:
	get_ctxt_fn get_ctxt;
	cg_service* cgs;
};

void register_builtin_typeconv(
	shared_ptr<type_converter> typeconv,
	shared_ptr<type_manager> typemgr
	)
{
	typedef function< void ( shared_ptr<node>, shared_ptr<node> ) > converter_t;

	shared_ptr<cgllvm_type_converter> cg_typeconv = shared_polymorphic_cast<cgllvm_type_converter>(typeconv);

	converter_t int2int_pfn = bind( &cgllvm_type_converter::int2int, cg_typeconv.get(), _1, _2 ) ;
	converter_t int2float_pfn = bind( &cgllvm_type_converter::int2float, cg_typeconv.get(), _1, _2 ) ;
	converter_t float2int_pfn = bind( &cgllvm_type_converter::float2int, cg_typeconv.get(), _1, _2 ) ;
	converter_t float2float_pfn = bind( &cgllvm_type_converter::float2float, cg_typeconv.get(), _1, _2 ) ;
	converter_t scalar2vec1_pfn = bind( &cgllvm_type_converter::scalar2vec1, cg_typeconv.get(), _1, _2 ) ;

	converter_t shrink_vec_pfn[5][5];
	for( int src_size = 1; src_size < 5; ++src_size ){
		for( int dest_size = 0; dest_size < 5; ++dest_size ){
			if( src_size > dest_size ){
				shrink_vec_pfn[src_size][dest_size] = bind(
					&cgllvm_type_converter::shrink_vector, cg_typeconv.get(),
					_1, _2, src_size, dest_size
					);
			}
		}
	}

	type_entry::id_t sint8_ts = typemgr->get( builtin_types::_sint8 );
	type_entry::id_t sint16_ts = typemgr->get( builtin_types::_sint16 );
	type_entry::id_t sint32_ts = typemgr->get( builtin_types::_sint32 );
	type_entry::id_t sint64_ts = typemgr->get( builtin_types::_sint64 );

	type_entry::id_t uint8_ts = typemgr->get( builtin_types::_uint8 );
	type_entry::id_t uint16_ts = typemgr->get( builtin_types::_uint16 );
	type_entry::id_t uint32_ts = typemgr->get( builtin_types::_uint32 );
	type_entry::id_t uint64_ts = typemgr->get( builtin_types::_uint64 );

	type_entry::id_t float_ts = typemgr->get( builtin_types::_float );
	type_entry::id_t double_ts = typemgr->get( builtin_types::_double );

	// type_entry::id_t bool_ts = typemgr->get( builtin_types::_boolean );

	cg_typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint8_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint8_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, sint8_ts, bool_ts, int2int_pfn );

	cg_typeconv->register_converter( type_converter::explicit_conv, sint16_ts, sint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint16_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint16_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, sint16_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint32_ts, sint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::warning_conv, sint32_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, sint32_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, sint32_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::warning_conv, sint64_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::warning_conv, sint64_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, sint64_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, uint8_ts, sint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, uint8_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint16_ts, uint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint16_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint16_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, uint16_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::better_conv, uint32_ts, sint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint32_ts, uint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::warning_conv, uint32_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, uint32_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, uint32_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint64_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint8_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint16_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint32_ts, int2int_pfn );
	cg_typeconv->register_converter( type_converter::warning_conv, uint64_ts, float_ts, int2float_pfn );
	cg_typeconv->register_converter( type_converter::warning_conv, uint64_ts, double_ts, int2float_pfn );
	// cg_typeconv->register_converter( type_converter::implicit_conv, uint64_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, sint8_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, sint16_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, sint32_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, sint64_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, uint8_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, uint16_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, uint32_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, float_ts, uint64_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::implicit_conv, float_ts, double_ts, float2float_pfn );
	// cg_typeconv->register_converter( type_converter::warning_conv, float_ts, bool_ts, default_conv );

	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, sint8_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, sint16_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, sint32_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, sint64_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, uint8_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, uint16_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, uint32_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, uint64_ts, float2int_pfn );
	cg_typeconv->register_converter( type_converter::explicit_conv, double_ts, float_ts, float2float_pfn );
	// cg_typeconv->register_converter( type_converter::warning_conv, double_ts, bool_ts, default_conv );

	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint8_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint16_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint32_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint64_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint8_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint16_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint32_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint64_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, float_ts, default_conv );
	//cg_typeconv->register_converter( type_converter::explicit_conv, bool_ts, double_ts, default_conv );

	//-------------------------------------------------------------------------
	// Register scalar <====> vector<scalar, 1>.
#define DEFINE_VECTOR_TYPE_IDS( btc ) \
	type_entry::id_t btc##_vts[5] = {-1, -1, -1, -1, -1};\
	btc##_vts[1] = typemgr->get( vector_of(builtin_types::btc , 1 ) ); \
	btc##_vts[2] = typemgr->get( vector_of(builtin_types::btc , 2 ) ); \
	btc##_vts[3] = typemgr->get( vector_of(builtin_types::btc , 3 ) ); \
	btc##_vts[4] = typemgr->get( vector_of(builtin_types::btc , 4 ) );

#define DEFINE_SHRINK_VECTORS( btc )				\
	for( int i = 1; i <=3; ++i ) {					\
		for( int j = i + 1; j <= 4; ++j ){			\
			cg_typeconv->register_converter(		\
				type_converter::explicit_conv,		\
				btc##_vts[j], btc##_vts[i],			\
				shrink_vec_pfn[j][i]				\
				);									\
		}											\
	}
	
#define DEFINE_VECTOR_AND_SHRINK( btc )	\
	DEFINE_VECTOR_TYPE_IDS( btc );	\
	DEFINE_SHRINK_VECTORS( btc );

	DEFINE_VECTOR_AND_SHRINK( _sint8 );
	DEFINE_VECTOR_AND_SHRINK( _sint16 );
	DEFINE_VECTOR_AND_SHRINK( _sint32 );
	DEFINE_VECTOR_AND_SHRINK( _sint64 );
	
	DEFINE_VECTOR_AND_SHRINK( _uint8 );
	DEFINE_VECTOR_AND_SHRINK( _uint16 );
	DEFINE_VECTOR_AND_SHRINK( _uint32 );
	DEFINE_VECTOR_AND_SHRINK( _uint64 );

	DEFINE_VECTOR_AND_SHRINK( _float );
	DEFINE_VECTOR_AND_SHRINK( _double );

}

shared_ptr<type_converter> create_type_converter(
	get_ctxt_fn const& get_ctxt,
	cg_service* cgs
	)
{
	return make_shared<cgllvm_type_converter>( get_ctxt, cgs );
}

END_NS_SASL_CODE_GENERATOR();