#ifndef MAKEFILE_H_INCLUDED
#define MAKEFILE_H_INCLUDED

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

#include <string>

class compile_config_item
{
public:
	compile_config_item(const std::string& id = "");

	std::string get_id();
	void set_id(const std::string& id);

private:
	std::string id_;
};

class search_dir : public compile_config_item
{
public:
	search_dir(const std::string& abspath, const std::string& id = std::string());

	std::string find(const std::string& pathstr) const;

private:
	boost::filesystem::path search_path_;
};

class work_env : public compile_config_item, public boost::noncopyable
{
	work_env(const work_env&);
public:
	work_env(const std::string& id = std::string());

	std::string	find_path(const std::string& pathstr) const;

	const std::vector<search_dir>& get_search_directories() const;
	std::vector<search_dir>& get_search_directories();

private:
	std::vector<search_dir>	search_dirs_;
};

class compile_flag : public compile_config_item
{
public:
	enum flag_types
	{
		FT_Flag = 0,
		FT_Group,
		FT_Flags
	};

	enum out_types{
		OT_CPP = 0,
		OT_BINARY,
		OT_LOG
	};

	compile_flag(const std::string& id = std::string());

	virtual std::string get_output_file() const;
	virtual void set_output_file(const std::string& file_name);

	virtual out_types get_output_type() const;
	virtual void set_output_type(out_types ot);

	virtual flag_types get_type() const;
private:
	std::string output_filename_;
	out_types ot_;
	std::string	id_;
};

class compile_flags : public compile_flag
{
public:
	const std::vector< boost::shared_ptr<compile_flag> >& get_flags() const;
	std::vector< boost::shared_ptr<compile_flag> >& get_flags();

	virtual compile_flag::flag_types get_type() const;
private:
	std::vector< boost::shared_ptr<compile_flag> > flags_;
};

class configuration : public compile_config_item
{

public:
	configuration(){}
	configuration(const configuration&){}
	configuration& operator = (const configuration&) { return *this; }

	typedef std::vector<std::string> input_list;

	compile_flags& get_flags();
	const compile_flags& get_flags() const;

	const work_env& get_work_environment() const;
	work_env& get_work_environment();

	const input_list& get_inputs() const;
	input_list& get_inputs();

private:
	compile_flags	flags_;
	work_env		env_;
	input_list		inputs_;
};

class makefile
{
public:
	typedef std::vector<configuration> config_list;
	static boost::shared_ptr<makefile> from_makefile(const std::string& file);
	static boost::shared_ptr<makefile> from_directory(const std::string& dir);

	const config_list& get_configurations() const;
	config_list& get_configurations();

private:
	config_list confs_;
};



#endif // MAKEFILE_H_INCLUDED
