#ifndef SASL_formatter_H
#define SASL_formatter_H

#include <string>
#include <vector>
#include <ostream>

class formatter{
public:
	friend class formatter_scope;
	virtual void output(const std::string& str) = 0;
	virtual void output_end_line() = 0;
protected:
	virtual void enter_scope() = 0;
	virtual void leave_scope() = 0;
};

class formatter_scope{
public:
	formatter_scope(formatter* pfmt):pfmt_(pfmt){
		pfmt_->enter_scope();
	}

	~formatter_scope(){
		pfmt_->leave_scope();
	}
private:
	void* operator new(size_t);
	formatter* pfmt_;
};

class default_formatter: public formatter{
public:
	default_formatter(
		std::ostream& ostr,
		const std::string& word_splitter,
		const std::string& line_splitter,
		const std::string& indentation
		);
	void output(const std::string& str);
	void output_end_line();

protected:
	void enter_scope();
	void leave_scope();

private:
	std::ostream& ostr_;
	std::string word_splitter_;
	std::string line_splitter_;
	std::string indentation_;

	std::string current_indentation_;
	std::vector<std::string> indentation_stack_;
};

#endif