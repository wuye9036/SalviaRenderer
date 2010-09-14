#ifndef SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_H
#define SASL_SEMANTIC_ANALYSER_SEMANTIC_INFOS_H

#include <sasl/include/semantic/semantic_forward.h>
#include <sasl/include/semantic/semantic_info.h>
#include <sasl/enums/type_types.h>
#include <sasl/enums/literal_constant_types.h>
#include <sasl/enums/buildin_type_code.h>
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
}

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::statement;

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

class const_value_semantic_info: public semantic_info{
public:
	typedef semantic_info base_type;
	const_value_semantic_info();

	void constant_value_literal( const std::string& litstr, literal_constant_types lctype);

	template <typename T> T value() const{
		return boost::get<T>(val);
	}
	template <typename T> void value( T val ){
		this->val = val;
	}

	buildin_type_code value_type() const;
	void value_type( buildin_type_code vtype );

private:
	boost::variant< unsigned long, long, double, std::string, bool, char > val;
	buildin_type_code valtype;
};

/*					
						has symbol		symbol's node		referred type		actual type
	buildin type		   no				N/A				   N/A				    this
	declaration	only	   yes			 first decl			 first decl			 full type
	definition             yes			 first decl			 first decl			 full type
	type ref/alias		   yes			 first decl          first decl			 full type
*/
class type_semantic_info: public semantic_info{
public:
	type_semantic_info();

	friend class semantic_info_collection;
	typedef semantic_info base_type;

	type_types type_type() const;
	void type_type( type_types ttype );

	// full_type returns back the raw type.
	// its behaviour is decided by ttype.
	boost::shared_ptr<type_specifier> full_type() const;

	void full_type( boost::shared_ptr<type_specifier> ftnode );
private:
	// ttype has 4-state: none, alias, buildin, composited.
	// none:
	//   it means is a null type that only declaration existed without definition.
	//   type_node is point to first declaration of this type.
	// alias:
	//   alias is a type definition symbol. type_node point to reference.
	// buildin:
	//   it is a buildin type symbol. buildin type point to the node which this symbol belongs to.
	// composited:
	//   all same as buildin but the type of type_node referred.
	type_types ttype;
	boost::weak_ptr<type_specifier> type_node;
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