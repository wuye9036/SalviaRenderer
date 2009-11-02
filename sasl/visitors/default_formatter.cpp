#include "formatters.h"

using namespace std;

default_formatter::default_formatter(
	ostream& ostr,
	const string& word_splitter,
	const string& sentence_splitter,
	const string& indentation
	)
	: ostr_(ostr),
	word_splitter_(word_splitter),
	line_splitter_(sentence_splitter),
	indentation_(indentation)
{}

void default_formatter::output(const std::string& str){
	ostr_ << str << word_splitter_;
}

void default_formatter::output_end_line(){
	ostr_ << line_splitter_ << current_indentation_;
}

void default_formatter::enter_scope(){
	indentation_stack_.push_back(current_indentation_);
	current_indentation_ += indentation_;
}

void default_formatter::leave_scope(){
	current_indentation_ = indentation_stack_.back();
	indentation_stack_.pop_back();
}