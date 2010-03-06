#ifndef SASL_ERROR_REPORTER_H
#define SASL_ERROR_REPORTER_H

#include <string>

class error_reporter_tags{
public:
	static std::string compiler_config_error();
	static std::string syntax_error();
};

/***
提供了Error Reportor的接口，并且提供了标准Error Reportor的格式的辅助字符串信息。
Error Report的实现中通常需要完成以下工作：
从其他途径或者依赖于调用方获得出错的上下文；
对上下文进行分析，并将Error Content格式化成可读和规范的形式。
所有函数均为并行执行函数。
***/
class error_reporter
{
public:
	/**
	报告编译器的内部错误。
	这些错误的发生多数是由编译器本身的BUG或不完善引起。
	**/
	virtual void report_compiler_internal_error(const std::string& error_content) = 0;

	/**
	报告被分析代码的语法错误
	**/
	virtual void report_syntax_error(const std::string& errContent) = 0;

	/**
	复制一个error_reporter，相同类型，但是初始参数为空。
	**/
	virtual error_reporter* clone() = 0;
};

#endif // error_reporter_H__