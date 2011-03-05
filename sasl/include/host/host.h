#ifndef SASL_HOST_HOST_H
#define SASL_HOST_HOST_H

#include <sasl/include/host/host_forward.h>

BEGIN_NS_SASL_HOST();

class shader_variable{
	template <typename T> void get( T& ) const;
	template <typename T> void set( T const& );
	std::string const & name() const;
};

class host_shader{
};

class host{
public:
	shader_variable variable( std::string const& name );

	// Entry
	void set_entry( std::string const& entry_name );

	// Execution
	void execute();
};

END_NS_SASL_HOST();

#endif