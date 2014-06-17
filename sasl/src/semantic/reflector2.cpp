#if 0

#include <sasl/include/semantic/reflector.h>

#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <salviar/include/rfile.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/utility/addressof.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using namespace sasl::syntax_tree;
using namespace sasl::utility;
using namespace salviar;

using eflib::fixed_string;

using boost::addressof;
using boost::make_shared;
using boost::shared_ptr;

using std::lower_bound;
using std::string;
using std::vector;

BEGIN_NS_SASL_SEMANTIC();

EFLIB_DECLARE_CLASS_SHARED_PTR(reflector);
EFLIB_DECLARE_CLASS_SHARED_PTR(reflection_impl);

class reflection_impl
{
public:
	// Friend for reflector could call compute_layout();
	friend class reflector;

	virtual eflib::fixed_string	entry_name() const;

	void	initialize(languages prof);
	
	reg_file* rfile(rfile_categories cat, uint32_t rfile_index)
	{
		auto& rfile = rfiles_[static_cast<uint32_t>(cat)];
		
		if( rfile_index < rfile.size() )
		{
			if(!rfile[rfile_index])
			{
				rfile[rfile_index].reset(new reg_file(cat, rfile_index));
			}
			return rfile[rfile_index].get();
		}
		else 
		{
			return nullptr;
		}
	}
	
	reg_file* rfile(reg_file_uid rfile)
	{
		return buffer(rfile.cat, rfile.index);
	}
	
	reg_file const* rfile(rfile_categories cat, uint32_t rfile_index) const
	{
		auto& rfile = rfiles_[static_cast<uint32_t>(cat)];
		
		if( rfile_index < rfile.size() )
		{
			return rfile[rfile_index].get();
		}
		else 
		{
			return nullptr;
		}
	}
	
	reg_file const* rfile(reg_file_uid rfile) const
	{
		return rfile(rfile.cat, rfile.index);
	}
	
	void update_reg_address()
	{
		for(uint32_t cat = static_cast<uint32_t>(rfile_categories::unknown); cat = static_cast<uint32_t>(rfile_categories::count); ++cat)
		{
			auto& rfile		= rfiles_[cat];
			auto& slot_buf_addr = buf_start_addr_[cat];
			
			uint32_t total = 0;
			for(auto& rfile: rfile)
			{
				slot_buf_addr.push_back(total);
				total += rfile->used_reg_count();
			}
			used_reg_count_[cat] = total;
		}
	}
	
	uint32_t used_reg_count(rfile_categories cat)
	{
		return used_reg_count_[static_cast<uint32_t>(cat)];
	}
	
	uint32_t reg_addr(reg_name const& rname)
	{
		uint32_t cat = static_cast<uint32_t>(rname.rfile.cat);
		return
			buf_start_addr_[cat][rname.rfile.index] +
			rfiles_[cat][rname.rfile.index]->reg_addr(rname);
	}
	

	// Impl specific members
	reflection_impl();

	void update_size(size_t sz, salviar::sv_usage usage);

	void module( module_semantic_ptr const& );
	bool is_module( module_semantic_ptr const& ) const;

	void entry(symbol*);
	bool is_entry(symbol*) const;
	
private:
	module_semantic*	module_sem_;
	symbol*				entry_point_;
	eflib::fixed_string	entry_point_name_;

	std::vector< std::unique_ptr<reg_file> >
				rfiles_			[static_cast<uint32_t>(rfile_categories::count)];
	uint32_t	used_reg_count_	[static_cast<uint32_t>(rfile_categories::count)];
	std::vector<uint32_t>
				buf_start_addr_	[static_cast<uint32_t>(rfile_categories::count)];		
};

class reflector
{
public:
	reflector(module_semantic* sem, eflib::fixed_string const& entry_name)
		: sem_(sem), current_entry_(NULL), reflection_(NULL), entry_name_(entry_name)
	{
	}

	reflector(module_semantic* sem)	: sem_(sem), current_entry_(NULL), reflection_(NULL)
	{
	}

	reflection_impl_ptr reflect()
	{
		if( !entry_name_.empty() )
		{
			vector<symbol*> overloads = sem_->root_symbol()->find_overloads(entry_name_);
			if ( overloads.size() != 1 )
			{
				return reflection_impl_ptr();
			}
			current_entry_ = overloads[0];
			return do_reflect();
		}
		else
		{
			symbol*				candidate = NULL;
			reflection_impl_ptr	candidate_reflection;
			BOOST_FOREACH( symbol* fn_sym, sem_->functions() )
			{
				current_entry_ = fn_sym;
				candidate_reflection = do_reflect();

				if(candidate_reflection)
				{
					if(candidate)
					{
						// TODO: More than one matched. conflict error.
						return reflection_impl_ptr();
					}
					candidate = fn_sym;
				}
			}
			return candidate_reflection;
		}
	}


private:
	reflection_impl_ptr do_reflect()
	{
		if( ! (sem_ && current_entry_) )
		{
			return reflection_impl_ptr();
		}

		salviar::languages lang = sem_->get_language();

		// Initialize language ABI information.
		reflection_impl_ptr ret = make_shared<reflection_impl>();
		ret->module_sem_			= sem_;
		ret->entry_point_		= current_entry_;
		ret->entry_point_name_	= current_entry_->mangled_name();
		reflection_ = ret.get();

		if( lang == salviar::lang_vertex_shader
			|| lang == salviar::lang_pixel_shader
			|| lang == salviar::lang_blending_shader
			)
		{
			// Process entry function.
			shared_ptr<function_def> entry_fn
				= current_entry_->associated_node()->as_handle<function_def>();
			assert( entry_fn );

			BOOST_FOREACH( shared_ptr<parameter> const& param, entry_fn->params )
			{
				if( !process_params(param, false, lang, semantic_value()) )
				{
					ret.reset();
					return ret;
				}
			}

			// Process global variables.
			BOOST_FOREACH( symbol* gvar_sym, sem_->global_vars() )
			{
				shared_ptr<declarator> gvar = gvar_sym->associated_node()->as_handle<declarator>();
				assert(gvar);

				// is_member is set to true for preventing aggregated variable.
				// And global variable only be treated as input.
				if( !add_semantic(gvar, true, false, lang, false) )
				{
					// If it is not attached to an valid semantic, it should be uniform variable.

					// Check the data type of global. Now global variables only support built-in types.
					node_semantic* psi = sem_->get_semantic( gvar.get() );
					if( psi->ty_proto()->is_builtin() || psi->ty_proto()->is_array()  )
					{
						ret->add_global_var(gvar_sym, psi->ty_proto()->as_handle<tynode>() );
					}
					else
					{
						//TODO: It an semantic error need to be reported.
						ret.reset();
						return ret;
					}
				}
			}
		}

		return ret;
	}

	bool process_registers(node_ptr const& v, bool is_member, bool is_global, semantic_value const* parent_sem)
	{
		assert(reflection_);
		node_semantic* pssi = sem_->get_semantic( v.get() );
		assert(pssi);	// TODO: Here are semantic analysis error.
		tynode* ptspec = pssi->ty_proto();
		assert(ptspec); // TODO: Here are semantic analysis error.

		salviar::semantic_value const& node_sem = pssi->semantic_value_ref();

		reg_file* rfile = nullptr;
		reflection_->rfile( is_global ? reg_file_uid::global() : reg_file_uid::params() );

		if( ptspec->is_builtin() )
		{
			builtin_types btc = ptspec->tycode;

			rfile->
#if 0
			sv_usage sem_s = semantic_usage( lang, is_output_semantic, node_sem );
			switch( sem_s ){

			case su_stream_in:
				return reflection_->add_input_semantic( node_sem, btc, true );
			case su_buffer_in:
				return reflection_->add_input_semantic( node_sem, btc, false );
			case su_stream_out:
				return reflection_->add_output_semantic( node_sem, btc, true );
			case su_buffer_out:
				return reflection_->add_output_semantic( node_sem, btc, false );
#endif
				assert( false );
				return false;
			}
		}
		else if( ptspec->node_class() == node_ids::struct_type )
		{
			// TODO: do not support nested aggregated variable. 
			struct_type* pstructspec = dynamic_cast<struct_type*>( ptspec );
			assert( pstructspec );
			BOOST_FOREACH( shared_ptr<declaration> const& decl, pstructspec->decls )
			{
				if ( decl->node_class() == node_ids::variable_declaration )
				{
					shared_ptr<variable_declaration> vardecl = decl->as_handle<variable_declaration>();
					BOOST_FOREACH( shared_ptr<declarator> const& dclr, vardecl->declarators )
					{
						if ( !process_varying(dclr, true, lang) )
						{
							assert( false );
							return false;
						}
					}
				}
			}

			return true;
		}

		return false;
	}

	module_semantic*	sem_;
	fixed_string		entry_name_;
	symbol*				current_entry_;
	reflection_impl*	reflection_;
};

reflection_impl_ptr reflect(module_semantic_ptr const& sem)
{
	reflector rfl( sem.get() );
	return rfl.reflect();
}

reflection_impl_ptr reflect(module_semantic_ptr const& sem, eflib::fixed_string const& entry_name)
{
	reflector rfl( sem.get(), entry_name );
	return rfl.reflect();
}

END_NS_SASL_SEMANTIC();

#endif
