#ifndef SASL_HOST_SHADER_LOG_IMPL_H
#define SASL_HOST_SHADER_LOG_IMPL_H

#include <sasl/include/host/host_forward.h>

#include <salviar/include/host.h>
#include <salviar/include/shader_object.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/tuple/tuple.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_HOST();

EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log_impl);
class shader_log_impl: public salviar::shader_log
{
public:
	virtual size_t				count() const;
	virtual std::string const&	log_string(size_t index) const;
	virtual void				append(std::string const&);
	
private:
	std::vector<std::string> logs_;
};

END_NS_SASL_HOST();

#endif