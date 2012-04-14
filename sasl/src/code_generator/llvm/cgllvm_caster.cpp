#include <eflib/include/platform/disable_warnings.h>
#include <LLVM/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>
#include <sasl/include/code_generator/llvm/cgllvm_caster.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/semantic/pety.h>
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
using ::sasl::semantic::caster_t;
using ::sasl::semantic::tid_t;
using ::sasl::semantic::pety_item_t;
using ::sasl::semantic::type_info_si;
using ::sasl::semantic::pety_t;

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

class cgllvm_caster : public caster_t{
public:
	cgllvm_caster( get_ctxt_fn get_ctxt, cg_service* cgs )
		: get_ctxt(get_ctxt), cgs(cgs)
	{
	}

	void store(shared_ptr<node> dest, shared_ptr<node> src, value_t const& v)
	{
		if( (dest->node_class().to_value() & node_ids::tynode.to_value()) != 0 ){
			// Overwrite source.
			get_ctxt(src)->value() = v;
		} else {
			// Store to dest.
			if( get_ctxt(dest)->value().storable() ){
				get_ctxt(dest)->value().store(v);
			} else {
				get_ctxt(dest)->value() = v;
			}
		}
	}
	// TODO if dest == src, maybe some bad thing happen ...
	void int2int( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );

		value_t casted = cgs->cast_ints(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);

		store(dest, src, casted);
	}

	void int2bool( shared_ptr<node> dest, shared_ptr<node> src ){
		if( src == dest ){ return; }
		value_t casted = cgs->cast_i2b( get_ctxt(src)->value() );
		store(dest, src, casted);
	}

	void int2float( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);
		
		assert( src_ctxt != dest_ctxt );

		value_t casted = cgs->cast_i2f(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);
		store( dest, src, casted );
	}

	void float2int( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );

		value_t casted = cgs->cast_f2i(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);
		cgs->store( dest_ctxt->value(), casted );
	}

	void float2float( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );

		value_t casted = cgs->cast_f2f(
			src_ctxt->get_rvalue(),
			dest_ctxt->data().tyinfo.get()
			);
		cgs->store( dest_ctxt->value(), casted );
	}

	void float2bool( shared_ptr<node> dest, shared_ptr<node> src ){
		if( src == dest ){ return; }
		value_t casted = cgs->cast_f2b( get_ctxt(src)->value() );
		store( dest, src, casted );
	}

	void scalar2vec1( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);
		assert( src_ctxt != dest_ctxt );
		store( dest, src, cgs->cast_s2v( src_ctxt->get_rvalue() ) );
	}

	void vec2scalar( shared_ptr<node> dest, shared_ptr<node> src ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);
		assert( src_ctxt != dest_ctxt );
		store( dest, src, cgs->cast_v2s( src_ctxt->get_rvalue() ) );
	}

	void shrink_vector( shared_ptr<node> dest, shared_ptr<node> src, int source_size, int dest_size ){
		cgllvm_sctxt* dest_ctxt = get_ctxt(dest);
		cgllvm_sctxt* src_ctxt = get_ctxt(src);

		assert( src_ctxt != dest_ctxt );
		assert( source_size > dest_size );

		value_t vector_value = dest_ctxt->get_rvalue();
		size_t swz_code = encode_sized_swizzle(dest_size);

		cgs->store(
			dest_ctxt->value(),
			vector_value.swizzle(swz_code)
			);
	}
private:
	get_ctxt_fn get_ctxt;
	cg_service* cgs;
};

void add_builtin_casts(
	shared_ptr<caster_t> caster,
	shared_ptr<pety_t> pety
	)
{
	typedef function< void ( shared_ptr<node>, shared_ptr<node> ) > cast_t;
	caster->set_tynode_getter(
		boost::bind(
		static_cast<shared_ptr<tynode> (pety_t::*)(tid_t)>(&pety_t::get),
		pety.get(), _1)
		);

	shared_ptr<cgllvm_caster> cg_caster = shared_polymorphic_cast<cgllvm_caster>(caster);

	cast_t int2int_pfn		= bind( &cgllvm_caster::int2int,	cg_caster.get(), _1, _2);
	cast_t int2bool_pfn		= bind( &cgllvm_caster::int2bool,	cg_caster.get(), _1, _2);
	cast_t int2float_pfn	= bind( &cgllvm_caster::int2float,	cg_caster.get(), _1, _2);
	cast_t float2int_pfn	= bind( &cgllvm_caster::float2int,	cg_caster.get(), _1, _2);
	cast_t float2float_pfn	= bind( &cgllvm_caster::float2float,cg_caster.get(), _1, _2);
	cast_t float2bool_pfn	= bind( &cgllvm_caster::float2bool,	cg_caster.get(), _1, _2);
	cast_t scalar2vec1_pfn	= bind( &cgllvm_caster::scalar2vec1,cg_caster.get(), _1, _2);
	cast_t vec2scalar_pfn	= bind( &cgllvm_caster::vec2scalar,	cg_caster.get(), _1, _2);
	cast_t shrink_vec_pfn[5][5];
	for( int src_size = 1; src_size < 5; ++src_size ){
		for( int dest_size = 0; dest_size < 5; ++dest_size ){
			if( src_size > dest_size ){
				shrink_vec_pfn[src_size][dest_size] = bind(
					&cgllvm_caster::shrink_vector, cg_caster.get(),
					_1, _2, src_size, dest_size
					);
			}
		}
	}

	tid_t sint8_ts  = pety->get( builtin_types::_sint8 );
	tid_t sint16_ts = pety->get( builtin_types::_sint16 );
	tid_t sint32_ts = pety->get( builtin_types::_sint32 );
	tid_t sint64_ts = pety->get( builtin_types::_sint64 );

	tid_t uint8_ts  = pety->get( builtin_types::_uint8 );
	tid_t uint16_ts = pety->get( builtin_types::_uint16 );
	tid_t uint32_ts = pety->get( builtin_types::_uint32 );
	tid_t uint64_ts = pety->get( builtin_types::_uint64 );

	tid_t float_ts  = pety->get( builtin_types::_float );
	tid_t double_ts = pety->get( builtin_types::_double );

	tid_t bool_ts   = pety->get( builtin_types::_boolean );

	cg_caster->add_cast_auto_prior( caster_t::imp, sint8_ts, sint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint8_ts, sint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint8_ts, sint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint8_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint8_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint8_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, sint16_ts, sint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint16_ts, sint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint16_ts, sint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint16_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint16_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint16_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, sint32_ts, sint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint32_ts, sint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint32_ts, sint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint32_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint32_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint32_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, sint64_ts, sint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint64_ts, sint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint64_ts, sint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint64_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint64_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, sint64_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, uint8_ts, sint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, sint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, sint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, sint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, uint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, uint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, uint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint8_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, uint16_ts, sint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint16_ts, sint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint16_ts, sint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint16_ts, sint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint16_ts, uint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint16_ts, uint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint16_ts, uint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint16_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint16_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint16_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, uint32_ts, sint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint32_ts, sint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint32_ts, sint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint32_ts, uint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint32_ts, uint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint32_ts, uint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint32_ts, sint64_ts, int2int_pfn );	
	cg_caster->add_cast_auto_prior( caster_t::imp, uint32_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint32_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint32_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint64_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint64_ts, uint8_ts,  int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint64_ts, uint16_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, uint64_ts, uint32_ts, int2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint64_ts, float_ts,  int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint64_ts, double_ts, int2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, uint64_ts, bool_ts,   int2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, sint8_ts,  float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, sint16_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, sint32_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, sint64_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, uint8_ts,  float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, uint16_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, uint32_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, float_ts, uint64_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, float_ts, double_ts, float2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, float_ts, bool_ts,   float2bool_pfn );

	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, sint8_ts,  float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, sint16_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, sint32_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, sint64_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, uint8_ts,  float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, uint16_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, uint32_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, uint64_ts, float2int_pfn );
	cg_caster->add_cast_auto_prior( caster_t::exp, double_ts, float_ts,  float2float_pfn );
	cg_caster->add_cast_auto_prior( caster_t::imp, double_ts, bool_ts,   float2bool_pfn );

	//cg_caster->add_cast( caster_t::exp, bool_ts, sint8_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, sint16_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, sint32_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, sint64_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, uint8_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, uint16_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, uint32_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, uint64_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, float_ts, default_conv );
	//cg_caster->add_cast( caster_t::exp, bool_ts, double_ts, default_conv );

	//-------------------------------------------------------------------------
	// Register scalar <====> vector<scalar, 1>.
#define DEFINE_VECTOR_TYPE_IDS( btc ) \
	tid_t btc##_vts[5] = {-1, -1, -1, -1, -1};\
	btc##_vts[1] = pety->get( vector_of(builtin_types::btc , 1 ) ); \
	btc##_vts[2] = pety->get( vector_of(builtin_types::btc , 2 ) ); \
	btc##_vts[3] = pety->get( vector_of(builtin_types::btc , 3 ) ); \
	btc##_vts[4] = pety->get( vector_of(builtin_types::btc , 4 ) );

#define DEFINE_SHRINK_VECTORS( btc )				\
	for( int i = 1; i <=3; ++i ) {					\
		for( int j = i + 1; j <= 4; ++j ){			\
			cg_caster->add_cast(		\
				caster_t::exp,		\
				btc##_vts[j], btc##_vts[i],			\
				shrink_vec_pfn[j][i]				\
				);									\
		}											\
	}												\
	cg_caster->add_cast( caster_t::eql,				\
		pety->get(builtin_types::btc),				\
		btc##_vts[1], scalar2vec1_pfn );			\
	cg_caster->add_cast( caster_t::eql,				\
		btc##_vts[1],								\
		pety->get(builtin_types::btc), vec2scalar_pfn );	
	
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

shared_ptr<caster_t> create_caster(
	get_ctxt_fn const& get_ctxt,
	cg_service* cgs
	)
{
	return boost::make_shared<cgllvm_caster>( get_ctxt, cgs );
}

END_NS_SASL_CODE_GENERATOR();