#include <sasl/include/semantic/reflector2.h>

#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <salviar/include/shader_reflection.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/addressof.hpp>
#include <boost/icl/interval_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using namespace sasl::syntax_tree;
using namespace sasl::utility;
using namespace salviar;

using eflib::fixed_string;

using boost::addressof;
using boost::make_shared;
using boost::shared_ptr;
using boost::icl::interval_set;
using boost::icl::continuous_interval;

using std::lower_bound;
using std::string;
using std::vector;
using std::map;
using std::pair;
using std::make_pair;
using std::deque;

BEGIN_NS_SASL_SEMANTIC();

EFLIB_DECLARE_CLASS_SHARED_PTR(reflector2);
EFLIB_DECLARE_CLASS_SHARED_PTR(reflection_impl2);

class reg_file;

enum class alloc_result: uint32_t
{
	ok = 0,
	out_of_reg_bound,
	register_has_been_allocated
};

class struct_layout
{
public:
	struct_layout(): total_size_(0)
	{
	}

	size_t add_member(size_t sz)
	{
		size_t offset = 0;
		if(total_size_ % 16 == 0)
		{
			offset = total_size_;
		}
		else if ( total_size_ + sz >= eflib::round_up(total_size_, REGISTER_SIZE) )
		{
			offset = eflib::round_up(total_size_, 16);
		}

		total_size_ = offset + sz;
		return offset;
	}

	size_t size() const
	{
		return eflib::round_up(total_size_, 16);
	}

private:
	size_t total_size_;
};

struct reg_handle
{
	reg_handle()
		: rfile(nullptr), v(0)
	{
	}

	reg_file*	rfile;
	size_t		v;
};

class reg_file
{
public:
	reg_file(reg_categories cat, uint32_t index)
		: uid_(cat, index), total_reg_count_(0)
	{
	}

	reg_name absolute_reg(reg_name const& rname) const
	{
		if(rname.rfile.cat == uid_.cat || rname.rfile.cat == reg_categories::offset)
		{
			return reg_name(uid_, rname.reg_index, rname.elem);
		}
		return reg_name();
	}

	alloc_result alloc_reg(size_t sz, reg_name const& rname)
	{	
		reg_name reg_end;
		return alloc_reg(reg_end, rname, sz);
	}

	reg_handle auto_alloc_reg(size_t sz)
	{
		auto_alloc_sizes_.push_back( static_cast<uint32_t>(sz) );
		reg_handle ret;
		ret.rfile = this;
		ret.v = auto_alloc_sizes_.size() - 1;
		return ret;
	}

	void update_auto_alloc_regs()
	{
		for(auto sz: auto_alloc_sizes_)
		{
			reg_name beg, end;
			alloc_auto_reg(beg, end, sz);
			auto_alloc_regs_.push_back(beg);
		}
	}

	void assign_semantic(reg_name const& beg, size_t sz, semantic_value const& sv_beg)
	{
		reg_sv_[beg] = sv_beg;

		if(sz > REGISTER_SIZE)
		{
			size_t reg_count = sz / REGISTER_SIZE;
			for(size_t reg_dist = 1; reg_dist < reg_count; ++reg_dist)
			{
				auto rname = beg.advance(reg_dist);
				reg_sv_[rname] = sv_beg.advance_index(reg_dist);
			}
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

	reg_name find_reg(reg_handle const& rh) const
	{
		return auto_alloc_regs_[rh.v];
	}

	void update_addr()
	{
		total_reg_count_ = 0;
		for(auto& range_addr: used_regs_)
		{
			auto& reg_range = range_addr;
			uint32_t reg_count = reg_range.upper().reg_index - reg_range.lower().reg_index;
			reg_addr_.insert( make_pair(reg_range.lower().reg_index, total_reg_count_) );
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
		return reg_addr_.at(iter->lower().reg_index) + (rname.reg_index - iter->lower().reg_index);
	}

private:
	alloc_result alloc_auto_reg(reg_name& beg, reg_name& end, size_t sz)
	{
		auto& slot_used_regs = used_regs_;
		beg = slot_used_regs.empty() ? reg_name(uid_, 0, 0) : slot_used_regs.rbegin()->upper().advance(1);
		return alloc_reg(end, beg, sz);
	}

	alloc_result alloc_reg(reg_name& end, reg_name const& beg, size_t sz)
	{
		if( 4 - beg.elem < sz / 4 )
		{
			return alloc_result::out_of_reg_bound;
		}
		end = beg.advance( eflib::round_up(sz, 16) / 16 );
		used_regs_.add( continuous_interval<reg_name>::right_open(beg, end) );
		return alloc_result::ok;
	}

	rfile_name						uid_;
	uint32_t						total_reg_count_;

	interval_set<reg_name>			used_regs_;
	map<uint32_t /*reg index*/, uint32_t /*addr*/>
									reg_addr_;

	map<node_semantic*, reg_name>	var_regs_;
	vector<uint32_t /*reg size*/>	auto_alloc_sizes_;
	vector<reg_name>				auto_alloc_regs_;

	map<semantic_value, reg_name>	sv_reg_;
	map<reg_name, semantic_value>	reg_sv_;
};

class reflection_impl2: public shader_reflection2
{
public:
	// Impl specific members
	reflection_impl2(module_semantic* mod_sem, symbol* entry)
		: module_sem_(mod_sem)
		, entry_fn_(entry)
		, entry_fn_name_(entry->mangled_name())
	{
		for(size_t i_cat = 0; i_cat < static_cast<uint32_t>(reg_categories::count); ++i_cat)
		{
			auto& cat_rfiles = rfiles_[i_cat];
			rfile_start_addr_[i_cat].resize(REG_CATEGORY_REGFILE_COUNTS[i_cat], 0);
			for(uint32_t i_rf = 0; i_rf < REG_CATEGORY_REGFILE_COUNTS[i_cat]; ++i_rf)
			{
				cat_rfiles.push_back( reg_file(static_cast<reg_categories>(i_cat), i_rf) );
			}

			used_reg_count_[i_cat] = 0;
		}
	}

	virtual languages language() const
	{
		return module_sem_->get_language();
	}

	virtual fixed_string entry_name() const override
	{
		return entry_fn_name_;
	}
	
	virtual vector<semantic_value> varying_semantics() const override
	{
		vector<semantic_value> ret;
		EFLIB_ASSERT_UNIMPLEMENTED();
		return ret;
	}

	virtual size_t available_reg_count(reg_categories cat) const override
	{
		return used_reg_count_[static_cast<uint32_t>(cat)];
	}

	virtual reg_name find_reg(reg_categories cat, semantic_value const& sv) const override
	{
		switch(cat)
		{
		case reg_categories::uniforms:
			return rfile( rfile_name::global() )->find_reg(sv);
		case reg_categories::varying:
			return rfile( rfile_name::varyings() )->find_reg(sv);
		default:
			return reg_name();
		}
	}

	virtual size_t reg_addr(reg_name const& rname) const override
	{
		uint32_t cat = static_cast<uint32_t>(rname.rfile.cat);
		return
			rfile_start_addr_[cat][rname.rfile.index] +
			rfiles_[cat][rname.rfile.index].reg_addr(rname);
	}

	reg_file const* rfile(rfile_name rfname) const
	{
		auto& cat_rfiles = rfiles_[static_cast<uint32_t>(rfname.cat)];
		return &cat_rfiles[rfname.index];
	}
	
	reg_file* rfile(rfile_name rfname)
	{
		auto& cat_rfiles = rfiles_[static_cast<uint32_t>(rfname.cat)];
		return &cat_rfiles[rfname.index];
	}

	void add_variable_reg(node_semantic const* var, reg_name const& rname)
	{
		input_var_regs_.insert( make_pair(var, rname) );
	}

	void add_variable_reg(node_semantic const* var, reg_handle const& rhandle)
	{
		var_auto_regs_.push_back( make_pair(var, rhandle) );
	}

	void assign_semantic(reg_name const& rname, size_t sz, semantic_value const& sv)
	{
		rfile(rname.rfile)->assign_semantic(rname, sz, sv);
	}

	void update_auto_alloc_regs()
	{
		for(auto& cat_rfiles: rfiles_)
		{
			for(auto& rfile: cat_rfiles)
			{
				rfile.update_auto_alloc_regs();
			}
		}

		for(auto& var_reg: var_auto_regs_)
		{
			add_variable_reg( var_reg.first, var_reg.second.rfile->find_reg(var_reg.second) );
		}

		var_auto_regs_.clear();
	}

	void update_reg_address()
	{
		for(uint32_t cat = static_cast<uint32_t>(reg_categories::unknown); cat < static_cast<uint32_t>(reg_categories::count); ++cat)
		{
			auto& cat_rfiles		= rfiles_[cat];
			auto& cat_rfiles_addr	= rfile_start_addr_[cat];
			
			cat_rfiles_addr.clear();
			uint32_t total = 0;

			for(auto& rfile: cat_rfiles)
			{
				rfile.update_addr();
				cat_rfiles_addr.push_back(total);
				total += rfile.used_reg_count();
			}

			used_reg_count_[cat] = total;
		}
	}

	reg_name find_reg(reg_handle rhandle) const
	{
		return rhandle.rfile->find_reg(rhandle);
	}

	void module( module_semantic_ptr const& );
	bool is_module( module_semantic_ptr const& ) const;

	void entry(symbol*);
	bool is_entry(symbol*) const;

private:
	module_semantic*		module_sem_;

	symbol*					entry_fn_;
	eflib::fixed_string		entry_fn_name_;

	vector<reg_file>	rfiles_				[static_cast<uint32_t>(reg_categories::count)];
	uint32_t				used_reg_count_		[static_cast<uint32_t>(reg_categories::count)];
	vector<uint32_t>	rfile_start_addr_	[static_cast<uint32_t>(reg_categories::count)];

	vector< pair<node_semantic const*, reg_handle> >
							var_auto_regs_;
	boost::unordered_map<node_semantic const*, reg_name>
							input_var_regs_;
};

class reflector2
{
public:
	reflector2(module_semantic* sem, eflib::fixed_string const& entry_name)
		: sem_(sem), current_entry_(NULL), reflection_(NULL), entry_name_(entry_name)
	{
	}

	reflector2(module_semantic* sem) : sem_(sem), current_entry_(NULL), reflection_(NULL)
	{
	}

	reflection_impl2_ptr reflect()
	{
		if( !entry_name_.empty() )
		{
			vector<symbol*> overloads = sem_->root_symbol()->find_overloads(entry_name_);
			if ( overloads.size() != 1 )
			{
				return reflection_impl2_ptr();
			}
			current_entry_ = overloads[0];
			return do_reflect();
		}
		else
		{
			symbol*					candidate = NULL;
			reflection_impl2_ptr	candidate_reflection;
			for( symbol* fn_sym: sem_->functions() )
			{
				current_entry_ = fn_sym;
				candidate_reflection = do_reflect();

				if(candidate_reflection)
				{
					if(candidate)
					{
						// TODO: More than one matched. conflict error.
						return reflection_impl2_ptr();
					}
					candidate = fn_sym;
				}
			}
			return candidate_reflection;
		}
	}

private:
	reflection_impl2_ptr do_reflect()
	{
		if( ! (sem_ && current_entry_) )
		{
			return reflection_impl2_ptr();
		}

		salviar::languages lang = sem_->get_language();

		// Initialize language ABI information.
		reflection_impl2_ptr ret = make_shared<reflection_impl2>(sem_, current_entry_);
		reflection_ = ret.get();

		switch(lang)
		{
		case salviar::lang_vertex_shader:
		case salviar::lang_pixel_shader:
		case salviar::lang_blending_shader:
			{
				// Process entry function.
				shared_ptr<function_def> entry_fn
					= current_entry_->associated_node()->as_handle<function_def>();
				assert(entry_fn);

				// 3 things to do:
				//		1. struct member offset computation;
				//		2. register allocation;
				//		3. semantic allocation;

				// Process parameters
				for(auto& param: entry_fn->params)
				{
					if( !process_entry_inputs(nullptr, param, input_types::param) )
					{
						//TODO: It an semantic error need to be reported.
						ret.reset();
						return ret;
					}
				}

				// Process global variables.
				for(symbol* gvar_sym: sem_->global_vars())
				{
					auto gvar = gvar_sym->associated_node()->as_handle<declarator>();
					if( !process_entry_inputs(nullptr, gvar, input_types::global) )
					{
						//TODO: It an semantic error need to be reported.
						ret.reset();
						return ret;
					}
				}

				// Process function output
				if( !process_entry_inputs(nullptr, entry_fn, input_types::output) )
				{
					//TODO: It an semantic error need to be reported.
					ret.reset();
					return ret;
				}

				reflection_->update_auto_alloc_regs();
				assign_semantics();
				reflection_->update_reg_address();
			}
		default:
			return ret;
		}
	}

	struct variable_info
	{
		size_t			offset;
		size_t			size;
		semantic_value	sv;					// only available for leaf.
		
		reg_name		reg;
		reg_handle		rhandle;

		variable_info*	parent;
		variable_info*	first_child;
		variable_info*	last_child;
		variable_info*	sibling;

		void initialize()
		{
			offset = 0;
			size = 0;
			parent = first_child = last_child = sibling = nullptr;
		}
	};

	enum class input_types
	{
		global,
		param,
		output
	};

	bool process_entry_inputs(
		variable_info*		parent_info,		// in and out
		node_ptr const&		v,
		input_types			input_type
		)
	{
		bool const is_member = (parent_info != nullptr);
		var_infos_.push_back( variable_info() );
		variable_info* minfo  = &var_infos_.back();
		minfo->initialize();
		minfo->parent = parent_info;

		assert(reflection_);
		node_semantic*	node_sem	= sem_->get_semantic( v.get() );
		tynode*			ty			= node_sem->ty_proto();

		rfile_name rf_name;

		switch(input_type)
		{
		case input_types::global:
			rf_name = rfile_name::global();
			break;
		case input_types::output:
			rf_name = rfile_name::outputs();
			break;
		case input_types::param:
			rf_name = ty->is_uniform() ? rfile_name::params() : rfile_name::varyings();
			break;
		default:
			EFLIB_ASSERT(false, "Invalid input type.");
			break;
		}

		reg_file* rfile = reflection_->rfile(rf_name);

		bool		use_parent_sv	= false;
		auto const* node_sv			= node_sem->semantic_value();
		auto		user_reg		= rfile->absolute_reg( node_sem->user_defined_reg() );

		// Member-special check: for register and semantic overwrite
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
				if( parent_info->sv.valid() )
				{
					// TODO: error: semantic rebounded
					return false;
				}
			}
			else
			{
				use_parent_sv = true;
				node_sv = &parent_info->sv;
			}
		}

		// For builtin type
		if( ty->is_builtin() )
		{
			// no-semantic bounded varying variables check
			if(node_sv == nullptr && input_type != input_types::global)
			{
				// TODO: error: no semantic bound on varying.
				return false;
			}

			minfo->size = reg_storage_size(ty->tycode);

			size_t reg_count = eflib::round_up(minfo->size, REGISTER_SIZE) / 16;
			if(is_member)
			{
				minfo->sv = *node_sv;
				if(use_parent_sv)
				{
					parent_info->sv = node_sv->advance_index(reg_count);
				}
			}
		}

		// For structure
		if( ty->node_class() == node_ids::struct_type )
		{
			// TODO: statistic total size, and member offset of structure via 'struct_layout'
			struct_type*	struct_ty = dynamic_cast<struct_type*>(ty);
			struct_layout	layout;
			for(auto const& decl: struct_ty->decls)
			{
				if ( decl->node_class() != node_ids::variable_declaration )
				{
					continue;
				}

				auto vardecl = decl->as_handle<variable_declaration>();
				for(auto const& dclr: vardecl->declarators)
				{
					process_entry_inputs(minfo, dclr, input_type);
					size_t offset = layout.add_member(minfo->last_child->size);
					minfo->last_child->offset = offset;
					sem_->get_semantic(dclr)->member_offset(offset);
				}
			}

			minfo->size = layout.size();
			if(use_parent_sv)
			{
				parent_info->sv = minfo->sv;
			}
		}

		if(parent_info)
		{  
			if(!parent_info->first_child)
			{
				parent_info->first_child = parent_info->last_child = minfo;
			}
			else
			{
				parent_info->last_child->sibling = minfo;
				parent_info->last_child = minfo;
			}
		}
		else
		{
			// Note: Register allocation only for top-level variable.
			if( user_reg.valid() )
			{
				if( rfile->alloc_reg(minfo->size, user_reg) != alloc_result::ok)
				{
					// TODO: error: register allocation failed. Maybe out of capacity or register has been allocated.
					return false;
				}
				reflection_->add_variable_reg(node_sem, user_reg);
				minfo->reg = user_reg;
			}
			else
			{
				auto rhandle = rfile->auto_alloc_reg(minfo->size);
				reflection_->add_variable_reg(node_sem, rhandle);
				minfo->rhandle = rhandle;
			}
		}

		return true;
	}

	void assign_semantics()
	{
		for(auto& var_info: var_infos_)
		{
			// Broadcast reg from root entry input to its members.
			if( var_info.rhandle.rfile != nullptr )
			{
				var_info.reg = reflection_->find_reg(var_info.rhandle);
			}

			if( !var_info.reg.valid() )
			{
				var_info.reg = var_info.parent->reg.advance(var_info.offset);
			}

			// Assign semantics to reg, only apply leaf nodes.
			if(var_info.first_child == nullptr && var_info.sv.valid())
			{
				reflection_->assign_semantic(var_info.reg, var_info.size, var_info.sv);
			}
		}
	}

	deque<variable_info>
						var_infos_;
	module_semantic*	sem_;
	fixed_string		entry_name_;
	symbol*				current_entry_;
	reflection_impl2*	reflection_;
};

salviar::shader_reflection2_ptr reflect2(module_semantic_ptr const& sem)
{
	reflector2 rfl( sem.get() );
	return rfl.reflect();
}

salviar::shader_reflection2_ptr reflect2(module_semantic_ptr const& sem, eflib::fixed_string const& entry_name)
{
	reflector2 rfl( sem.get(), entry_name );
	return rfl.reflect();
}

END_NS_SASL_SEMANTIC();
