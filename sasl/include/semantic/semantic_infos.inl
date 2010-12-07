#ifndef SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_INL
#define SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_INL

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/semantic/semantic_info.h>
#include <sasl/include/semantic/type_manager.h>
#include <sasl/enums/default_hasher.h>
#include <sasl/enums/type_types.h>
#include <sasl/enums/literal_constant_types.h>
#include <sasl/enums/buildin_type_code.h>
#include <sasl/enums/enums_helper.h>
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

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::statement;

//////////////////////////////////////////////////////////////////////////
// Global semantic infos

class global_si: public semantic_info{
public:
	typedef semantic_info base_type;

	global_si();

	boost::shared_ptr<class type_manager> type_manager() const;
	boost::shared_ptr<symbol> root() const;
	boost::shared_ptr< ::sasl::common::compiler_info_manager > compiler_infos() const;

private:
	boost::shared_ptr<class type_manager> typemgr;
	boost::shared_ptr<symbol> rootsym;
	boost::shared_ptr< ::sasl::common::compiler_info_manager > compinfo;
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

	static boost::shared_ptr<type_specifier> from_node( ::boost::shared_ptr<node> );
};

#define SASL_TYPE_INFO_PROXY()	\
	private:	\
		type_info_si_impl type_info_proxy;	\
	public:	\
		virtual type_entry::id_t entry_id() const { return type_info_proxy.entry_id(); }	\
		virtual void entry_id( type_entry::id_t id ) { type_info_proxy.entry_id( id ); }	\
		virtual ::boost::shared_ptr< type_specifier > type_info() const{ return type_info_proxy.type_info(); }	\
		virtual void type_info( ::boost::shared_ptr< type_specifier > typespec, ::boost::shared_ptr<symbol> sym ) { type_info_proxy.type_info( typespec, sym ); }

class type_info_si_impl: public type_info_si{
public:
	type_info_si_impl( boost::shared_ptr<type_manager> typemgr );

	virtual type_entry::id_t entry_id() const;
	virtual void entry_id( type_entry::id_t id );

	virtual ::boost::shared_ptr< type_specifier > type_info() const;
	virtual void type_info( ::boost::shared_ptr< type_specifier >, boost::shared_ptr<symbol> );

private:
	type_entry::id_t tid;
	::boost::weak_ptr< class type_manager > typemgr;
};

class const_value_si: public type_info_si{
public:
	typedef semantic_info base_type;
	const_value_si( boost::shared_ptr<type_manager> typemgr );

	void set_literal( const std::string& litstr, literal_constant_types lctype);

	SASL_TYPE_INFO_PROXY();

	template <typename T> T value() const;
	template <typename T> void value( T val );

	buildin_type_code value_type() const;
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

	SASL_TYPE_INFO_PROXY();
};

class variable_semantic_info: public semantic_info{
public:
	friend class semantic_info_collection;
	typedef semantic_info base_type;

	bool is_local() const;
	void is_local( bool isloc );
private:
	variable_semantic_info();
	bool isloc;
};

class execution_block_semantic_info: public semantic_info{
public:
	friend class semantic_info_collection;
	boost::shared_ptr<struct node> execute_point( size_t id ) const;
	void append( boost::shared_ptr<struct node> execute_point );

private:
	execution_block_semantic_info();
	std::vector< boost::weak_ptr<struct node> > execute_points;
};

class execute_point_semantic_info: public semantic_info{
public:
	void id( size_t stmt_id );
	size_t id();
	::std::string execute_point_name();

	boost::shared_ptr<struct node> execute_point() const;

private:
	execute_point_semantic_info();
	size_t stmt_id; // tag of statement, for jumping.
	boost::weak_ptr< execution_block_semantic_info > root;
};

END_NS_SASL_SEMANTIC();

#endif
