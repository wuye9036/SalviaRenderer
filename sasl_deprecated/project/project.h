#ifndef PROJECT_H_INCLUDED
#define PROJECT_H_INCLUDED

#include <string>
#include <vector>

class preprocessor;
class error_reporter;
class makefile;
class configuration;
class unit;

#include <boost/smart_ptr.hpp>

/***
一个工程是一个完整编译过程的最小单位。它包含了一或多个源文件的路径名以及若干组属于不同配置的编译器参数。
每个编译器应用程序实例只能启用一个工程。
编译器架构上支持文件级别的并行语法和语义分析。但是连接阶段仍然只能串行完成。
目前的实现为单线程方式。
***/
class project
{
public:
	static project& instance();

	/**
	设置preprocessor和ErrorReport执行初始化过程。
	该函数在其他方法被调用前至少需要被执行一次。
	**/
	static void initialize(boost::shared_ptr<preprocessor> pp, boost::shared_ptr<error_reporter> err_reporter);
	std::vector<boost::shared_ptr<unit> >& get_units();

	/**
		对框架进行语法分析，获得相应的语法树。
		如果传入config_name，则会选择相应的配置进行分析；
		如果传入的config_name不存在，或者分析过程出现错误，则分析自动停止。
		如果传入的config_name为空，则选取工程的第一个Config。
	**/
	void parse(const std::string& config_name = "");

	///返回Project的preprocessor
	preprocessor* get_pp();

	///返回Project的Error Reporter
	error_reporter* get_error_reporter();

private:
	/**
	获取一个已知ID的编译器配置。如果配置文件不存在则返回空。
	函数由框架调用。配置名称来自于saslc命令行的配置名。
	**/
	configuration* get_config(const std::string& config_name);

	///根据传入的配置进行分析。
	bool parse_units(configuration* pconf);

	///链接，生成目标代码。
	bool link(configuration* pconf);

	project();
	~project();

	static project prj_;

	boost::shared_ptr<preprocessor>			pp_;
	boost::shared_ptr<error_reporter>		err_reporter_;
	boost::shared_ptr<makefile>				mf_;
	std::vector<boost::shared_ptr<unit> >	units_;
};

//project::Instance的一个简化调用封装。
inline project& proj()
{
	return project::instance();
}

#endif // PROJECT_H_INCLUDED
