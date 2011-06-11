#ifndef SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_H
#define SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/semantic/semantic_info.h>
#include <sasl/include/semantic/type_manager.h>
#include <sasl/enums/default_hasher.h>
#include <sasl/enums/type_types.h>
#include <sasl/enums/literal_constant_types.h>
#include <sasl/enums/builtin_type_code.h>
#include <sasl/enums/enums_helper.h>

#include <salviar/include/enums.h>

#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace sasl {
	namespace syntax_tree{
		struct type_specifier;
		struct statement;
		struct node;
	}
	namespace common{
		class compiler_info_manager;
	}
}

namespace softart{
	enum languages;
}

BEGIN_NS_SASL_SEMANTIC();

int32_t swizzle_field_name_to_id( char ch );
// bool is_valid_swizzle_field( char ch, int max_size );
int32_t encode_swizzle( char _1st, char _2nd = 0, char _3rd = 0, char _4th = 0 );
int32_t encode_swizzle( int& dest_size, int& min_src_size, char const* masks );
int32_t encode_sized_swizzle( int size );
// char decode_swizzle( int32_t swz, int pos );

using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::statement;

//////////////////////////////////////////////////////////////////////////
// Global semantic infos

class module_si: public semantic_info{
public:
	typedef semantic_info base_type;

	module_si();

	boost::shared_ptr<class type_manager> type_manager() const;
	boost::shared_ptr<symbol> root() const;
	boost::shared_ptr< ::sasl::common::compiler_info_manager > compiler_infos() const;

	std::vector< boost::shared_ptr<symbol> > const& globals() const;
	std::vector< boost::shared_ptr<symbol> >& globals();

	std::vector< boost::shared_ptr<symbol> > const& functions() const;
	std::vector< boost::shared_ptr<symbol> >& functions();

private:
	boost::shared_ptr<class type_manager> typemgr;
	boost::shared_ptr<symbol> rootsym;
	boost::shared_ptr< ::sasl::common::compiler_info_manager > compinfo;

	std::vector< boost::shared_ptr<symbol> > gvars;
	std::vector< boost::shared_ptr<symbol> > fns;
};

//////////////////////////////////////
//  program semantic infos.
class program_si : public semantic_info{
public:
	typedef semantic_info base_type;
	program_si( );

	const std::string& name() const;
	void name( const std::string& );

private:
	program_si( const program_si& );
	program_si& operator = (const program_si& );

	std::string prog_name;
};

class type_info_si: public semantic_info{
public:
	virtual type_entry::id_t entry_id() const = 0;
	virtual void entry_id( type_entry::id_t id ) = 0;

	virtual ::boost::shared_ptr< type_specifier > type_info() const = 0;
	virtual void type_info( ::boost::shared_ptr< type_specifier >, ::boost::shared_ptr<symbol> sym ) = 0;
	virtual void type_info( builtin_type_code btc ) = 0;

	static boost::shared_ptr<type_specifier> from_node( ::boost::shared_ptr<node> );
};

#define SASL_TYPE_INFO_PROXY()	\
	private:	\
		type_info_si_impl type_info_proxy;	\
	public:	\
		virtual type_entry::id_t entry_id() const { return type_info_proxy.entry_id(); }	\
		virtual void entry_id( type_entry::id_t id ) { type_info_proxy.entry_id( id ); }	\
		virtual ::boost::shared_ptr< type_specifier > type_info() const{ return type_info_proxy.type_info(); }	\
		virtual void type_info( ::boost::shared_ptr< type_specifier > typespec, ::boost::shared_ptr<symbol> sym ) { type_info_proxy.type_info( typespec, sym ); } \
		virtual void type_info( builtin_type_code btc ){ type_info_proxy.type_info(btc); }

class type_info_si_impl: public type_info_si{
public:
	type_info_si_impl( boost::shared_ptr<type_manager> typemgr );

	virtual type_entry::id_t entry_id() const;
	virtual void entry_id( type_entry::id_t id );

	virtual ::boost::shared_ptr< type_specifier > type_info() const;
	virtual void type_info( ::boost::shared_ptr< type_specifier >, boost::shared_ptr<symbol> );
	virtual void type_info( builtin_type_code btc );
private:
	type_entry::id_t tid;
	::boost::weak_ptr< class type_manager > typemgr;
};

class const_value_si: public type_info_si{
public:
	typedef semantic_info base_type;
	const_value_si( boost::shared_ptr<type_manager> typemgr );

	void set_literal( const std::string& litstr, literal_constant_types lctype );

	SASL_TYPE_INFO_PROXY();

	template <typename T> T value() const;
	template <typename T> void value( T val );

	builtin_type_code value_type() const;
private:
	boost::variant< uint64_t, int64_t, double, std::string, bool, char > val;
};

class type_si : public type_info_si{
public:
	type_si( boost::shared_ptr<type_manager> typemgr );

	SASL_TYPE_INFO_PROXY();
};

class storage_si: public type_info_si{

public:
	storage_si( boost::shared_ptr<type_manager> typemgr );

	salviar::semantic get_semantic() const;
	void set_semantic( salviar::semantic v );

	// For members
	int mem_index() const;
	void mem_index( int i );

	// Indicate that member is a swizzle
	//	e.g. float2.xyyx;
	int32_t swizzle() const;
	void swizzle( int32_t v );

	SASL_TYPE_INFO_PROXY();

private:
	int memidx;
	int32_t swz;
	salviar::semantic sem;
};

/*! \brief fnvar_si storages the semantic informations of variable of function.
 *
 */
class fnvar_si: public semantic_info{
public:
	fnvar_si();

	void scope( boost::shared_ptr<symbol> const& );
	symbol* scope() const;

	void name( std::string const& );
	std::string const& name() const;

private:
	symbol* sym;
	std::string fn_name;
};

class statement_si: public semantic_info{

public:

	statement_si();
	
	const std::string& exit_point() const;
	void exit_point( const std::string& );

	const std::string& loop_point() const;
	void loop_point( const std::string& );

	bool has_loop() const;
	void has_loop( bool v );

	boost::shared_ptr<node> parent_block() const;
	void parent_block( boost::shared_ptr<node> );
private:
	bool has_lp;

	std::string loop_pt;
	std::string exit_pt; // for if, while, for do-while.

	boost::weak_ptr<node> parent;
};

END_NS_SASL_SEMANTIC();

#endif
