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

// Add casters for scalar, vector, matrix.
void add_svm_casters(
	shared_ptr<caster_t> const& caster, caster_t::casts casts, 
	builtin_types src, builtin_types dst,
	caster_t::cast_t fn,	pety_t* pety
	)
{
	caster->add_cast_auto_prior( casts,	pety->get(src),	pety->get(dst), fn );
	for( size_t i = 1; i <= 4; ++i )
	{
		caster->add_cast_auto_prior( casts,
			pety->get( vector_of(src, i) ),	pety->get( vector_of(dst, i) ),
			fn );
		for( size_t j = 1; j <= 4; ++j )
		{
			caster->add_cast_auto_prior( casts,
				pety->get( matrix_of(src, i, j) ),	pety->get( matrix_of(dst, i, j) ),
				fn );
		}
	}
}

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

	builtin_types sint8_bt	= builtin_types::_sint8	;
	builtin_types sint16_bt	= builtin_types::_sint16;
	builtin_types sint32_bt	= builtin_types::_sint32;
	builtin_types sint64_bt	= builtin_types::_sint64;

	builtin_types uint8_bt	= builtin_types::_uint8	;
	builtin_types uint16_bt	= builtin_types::_uint16;
	builtin_types uint32_bt	= builtin_types::_uint32;
	builtin_types uint64_bt	= builtin_types::_uint64;

	builtin_types float_bt	= builtin_types::_float	;
	builtin_types double_bt	= builtin_types::_double;

	builtin_types bool_bt	= builtin_types::_boolean;

	add_svm_casters( cg_caster, caster_t::imp, sint8_bt, sint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint8_bt, sint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint8_bt, sint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint8_bt, uint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint8_bt, uint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint8_bt, uint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint8_bt, uint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint8_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint8_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint8_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, sint16_bt, sint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint16_bt, sint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint16_bt, sint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint16_bt, uint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint16_bt, uint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint16_bt, uint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint16_bt, uint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint16_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint16_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint16_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, sint32_bt, sint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint32_bt, sint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint32_bt, sint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint32_bt, uint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint32_bt, uint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint32_bt, uint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint32_bt, uint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint32_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint32_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint32_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, sint64_bt, sint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint64_bt, sint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint64_bt, sint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint64_bt, uint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint64_bt, uint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint64_bt, uint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, sint64_bt, uint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint64_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint64_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, sint64_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, uint8_bt, sint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, sint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, sint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, sint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, uint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, uint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, uint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint8_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, uint16_bt, sint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint16_bt, sint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint16_bt, sint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint16_bt, sint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint16_bt, uint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint16_bt, uint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint16_bt, uint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint16_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint16_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint16_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, uint32_bt, sint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint32_bt, sint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint32_bt, sint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint32_bt, uint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint32_bt, uint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint32_bt, uint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint32_bt, sint64_bt, int2int_pfn,	pety.get() );	
	add_svm_casters( cg_caster, caster_t::imp, uint32_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint32_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint32_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, uint64_bt, sint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint64_bt, sint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint64_bt, sint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint64_bt, sint64_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint64_bt, uint8_bt,  int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint64_bt, uint16_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, uint64_bt, uint32_bt, int2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint64_bt, float_bt,  int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint64_bt, double_bt, int2float_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, uint64_bt, bool_bt,   int2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, float_bt, sint8_bt,  float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, float_bt, sint16_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, float_bt, sint32_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, float_bt, sint64_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, float_bt, uint8_bt,  float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, float_bt, uint16_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, float_bt, uint32_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, float_bt, uint64_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, float_bt, double_bt, float2float_pfn,pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, float_bt, bool_bt,   float2bool_pfn,	pety.get() );

	add_svm_casters( cg_caster, caster_t::exp, double_bt, sint8_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, sint16_bt,float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, sint32_bt,float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, sint64_bt,float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, uint8_bt, float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, uint16_bt,float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, uint32_bt,float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, uint64_bt,float2int_pfn,	pety.get() );
	add_svm_casters( cg_caster, caster_t::exp, double_bt, float_bt, float2float_pfn,pety.get() );
	add_svm_casters( cg_caster, caster_t::imp, double_bt, bool_bt,  float2bool_pfn,	pety.get() );

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

	DEFINE_VECTOR_AND_SHRINK( _boolean );
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