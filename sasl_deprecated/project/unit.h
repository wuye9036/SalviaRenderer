#ifndef SASL_UNIT_H
#define SASL_UNIT_H

#include <string>
#include <boost/smart_ptr.hpp>

class context;
class scope;
class error_reporter;

/**
编译单元，一般是单个源文件。
**/
class unit
{
public:
	unit();

	///	获得当前的上下文。
	context* get_context();

	///	获得全局作用域。
	scope* get_global_scope();

	/// 分析一个Unit
	bool parse(const std::string& filename);

	/// 返回相应的Error Report
	error_reporter* get_error_reporter();

private:
	boost::shared_ptr<context>			hcontext_;
	boost::shared_ptr<scope>			hscope_;
	boost::shared_ptr<error_reporter>	herror_reporter_;
};

#endif // unit_H__