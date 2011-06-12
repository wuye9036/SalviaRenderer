#ifndef SALVIAR_SHADER_COMPILER_H
#define SALVIAR_SHADER_COMPILER_H

#include <salviar/include/salviar_forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <map>

BEGIN_NS_SALVIAR();

class shader_code;

typedef boost::function<std::string ( std::string const& )> include_hook_t;

class compiler{
public:
	void defines( std::map<std::string, std::string> const& defs );
	boost::shared_ptr<shader_code> compile( std::string const& code );
};

extern "C"{
	boost::shared_ptr<compiler> salvia_create_compiler();
}

END_NS_SALVIAR();

#endif