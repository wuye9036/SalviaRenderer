#ifndef SASL_COMPILER_OPTIONS_H
#define SASL_COMPILER_OPTIONS_H

#include <sasl/include/compiler/compiler_forward.h>

BEGIN_NS_SASL_COMPILER();

namespace po = boost::program_options;

class options_filter{
public:
	virtual void execute( bool& could_return_back ) = 0;
	virtual void set_options( po::variables_map const& vm ) = 0;
};

class options_display_info: public options_filter{
public:
	void execute( bool& could_return_back );
	void set_options( po::variables_map const & vm );
};

class options_manager{
public:
	options_manager();
	static options_manager& instance();
	
	options_display_info const & display_info() const;
	
	po::variables_map const & variables() const;
	
private:
	options_display_info disp_info;
	po::variable_maps vm;
};

END_NS_SASL_COMPILER();

#endif