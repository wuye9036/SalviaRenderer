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

enum class alloc_result: uint32_t
{
	ok = 0,
	register_has_been_allocated
};

struct rfile_impl: public reg_file
{
public:
	typedef boost::icl::interval_map<reg_name, uint32_t /*physical_reg*/>
										reg_addr_map;
	typedef reg_addr_map::interval_type	reg_interval;

public:
	rfile_impl(rfile_categories cat, uint32_t index)
		: uid_(cat, index), total_reg_count_(0)
	{
	}
	
	alloc_result alloc_reg(size_t sz, reg_name const& rname)
	{	
		reg_name reg_end;
		return alloc_reg(reg_end, rname, sz);
	}
	
	alloc_result alloc_reg(size_t sz, std::string const& vname, reg_name const& rname)
	{
		auto const& reg_beg = rname;
		auto reg_end = rname.advance( eflib::round_up(sz, 16) / 16 );
		auto reg_range = reg_interval::right_open(reg_beg, reg_end);
		
		return assign_variable(reg_range, vname) ? alloc_result::ok : alloc_result::register_has_been_allocated;
	}
	
	alloc_result alloc_reg(size_t sz, std::string const& vname)
	{
		pending_vars_.push_back( std::make_pair( vname, static_cast<uint32_t>(sz) ) );
	}
	
	void update_unspecified_reg_variable()
	{
		for(auto const& var_sz: pending_vars_)
		{
			reg_name beg, end;
			alloc_reg(beg, end, var_sz.second);
			assign_variable( reg_interval(beg, end), var_sz.first);
		}
	}
	
	void assign_semantic(reg_name const& beg, reg_name const& end, semantic_value const& sv_beg)
	{
		reg_sv_[beg] = sv_beg;
		
		for( uint32_t rindex = beg.reg_index+1; rindex <= end.reg_index; ++rindex)
		{
			auto rname = reg_name(beg.rf, rindex, 0);
			reg_sv_[rname] = sv_beg.advance_index(rindex - beg.reg_index);
		}
	}
	
	reg_name find_reg(semantic_value const& sv) const
	{
		auto iter = sv_reg_.find(sv);
		if(iter == sv_reg_.end())
		{
			return reg_name();
		}
		return iter->second;
	}
	
	reg_interval
			 find_reg(std::string    const& vname) const
	{
		auto iter = var_reg_.find(vname);
		if( iter == var_reg_.end() )
		{
			return reg_interval();
		}
		return iter->second;
	}
	
	void update_addr()
	{
		total_reg_count_ = 0;
		for(auto& range_addr: used_regs_)
		{
			auto& reg_range = range_addr.first;
			uint32_t reg_count = reg_range.upper().reg_index - reg_range.lower().reg_index;
			range_addr.second = total_reg_count_;
			total_reg_count_ += reg_count;
		}
	}
	
	uint32_t used_reg_count() const
	{
		return total_reg_count_;
	}
	
	uint32_t reg_addr(reg_name const& rname) const
	{
		auto iter = used_regs_.find(rname);
		if( iter == used_regs_.end() )
		{
			return static_cast<uint32_t>(-1);
		}
		return iter->second + (rname.reg_index - iter->first.lower().reg_index);
	}

private:
	alloc_result alloc_reg(reg_name& beg, reg_name& end, size_t sz)
	{
		auto& slot_used_regs = used_regs_;
		beg = slot_used_regs.empty() ? reg_name(uid_, 0, 0) : slot_used_regs.rbegin()->first.upper().advance(1);
		return alloc_reg(end, beg, sz);
	}
	
	alloc_result alloc_reg(reg_name& end, reg_name const& beg, size_t sz)
	{
		end = beg.advance( eflib::round_up(sz, 16) / 16 );
		used_regs_.add( std::make_pair(reg_interval::right_open(beg, end), 0) );
		return alloc_result::ok;
	}
	
	bool assign_variable(reg_interval const& reg_range, std::string const& vname)
	{
		if( boost::icl::intersects(used_regs_, reg_range) )
		{
			return false;
		}
		
		var_reg_[vname] = reg_range;
		return true;
	}
	
	reg_file_uid						uid_;
	uint32_t							total_reg_count_;

	reg_addr_map						used_regs_;
	std::vector< std::pair<std::string /*var name*/, uint32_t /*size*/> >
										pending_vars_;
	std::map<semantic_value, reg_name>	sv_reg_;
	std::map<reg_name, semantic_value>	reg_sv_;
	std::map<std::string, reg_interval>	var_reg_;
	std::map<uint32_t /*reg index*/, uint32_t /*addr*/>
										reg_addr_;
};

class reflection_impl
{
public:
	// Friend for reflector could call compute_layout();
	friend class reflector;

	virtual eflib::fixed_string	entry_name() const;

	void	initialize(languages prof);
	
	rfile_impl* rfile(rfile_categories cat, uint32_t rfile_index)
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
	
	rfile_impl* rfile(reg_file_uid rfile)
	{
		return buffer(rfile.cat, rfile.index);
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

	std::vector<rfile_impl>
				rfiles_				[static_cast<uint32_t>(rfile_categories::count)];
	uint32_t	used_reg_count_		[static_cast<uint32_t>(rfile_categories::count)];
	std::vector<uint32_t>
				rfile_start_addr_	[static_cast<uint32_t>(rfile_categories::count)];		
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

	bool process_registers(node_ptr const& v, bool is_member, bool is_global, semantic_value const* sv, * semantic_value* updated_sv)
	{
		assert(reflection_);
		node_semantic* node_sem = sem_->get_semantic( v.get() );
		assert(node_sem);		// TODO: Here are semantic analysis error.
		tynode* ty = node_sem->ty_proto();
		assert(ty);				// TODO: Here are semantic analysis error.

		rfile_impl* rfile =
			reflection_->rfile(
				is_global
				? reg_file_uid::global() 
				: ty->is_uniform() ? reg_file_uid::params() : reg_file_uid::varyings()
			);
		auto const* node_sv	 = node_sem->semantic_value();
		auto		user_reg = node_sem->user_defined_reg();

		if(is_member)
		{
			if( ty->is_uniform() )
			{
				// TODO: error function parameters cannot be declared 'uniform'
				return false;
			}

			if ( node_sem->user_defined_reg().valid() )
			{
				// TODO: structure member cannot specify position
				return false;
			}

			if ( node_sv->valid() )
			{
				if( sv->valid() )
				{
					// TODO: error: semantic rebounded
					return false;
				}
			}
			else
			{
				node_sv = sv;
			}
		}
		
		if(node_sv == nullptr && !is_global)
		{
			// TODO: error: no semantic bound on varying.
			return false;
		}

		size_t	var_size = 0;

		if( ty->is_builtin() )
		{
			var_size = reg_storage_size(ty->tycode);
			rfile->alloc_reg(
			return true;
		}

		if( ty->node_class() == node_ids::struct_type )
		{
			// TODO: statistic total size, and member offset of structure via 'struct_layout'
			struct_type* struct_ty = dynamic_cast<struct_type*>(ty);
			assert(struct_ty);

			semantic_value child_sv, updated_child_sv;
			if(sv != nullptr)
			{
				child_sv = *sv;
			}

			BOOST_FOREACH( shared_ptr<declaration> const& decl, struct_ty->decls )
			{
				if ( decl->node_class() == node_ids::variable_declaration )
				{
					shared_ptr<variable_declaration> vardecl = decl->as_handle<variable_declaration>();
					BOOST_FOREACH( shared_ptr<declarator> const& dclr, vardecl->declarators )
					{
						if(sv != nullptr)
						{
							process_registers(dclr, true, is_global, &child_sv, &updated_child_sv);
						}
						else
						{
							process_registers(dclr, true, is_global, nullptr, nullptr);
						}
					}
				}
			}

			if(sv != nullptr)
			{
				assert(updated_sv != nullptr);
				*updated_sv = *sv;
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
