#ifndef EFLIB_DIAGNOSTICS_LOG_SERIALIZER_H
#define EFLIB_DIAGNOSTICS_LOG_SERIALIZER_H

#include <eflib/include/io/stream.h>
#include <eflib/include/string/string.h>

#include <boost/tuple/tuple.hpp>

#include <vector>
#include <fstream>
#include <ostream>

namespace eflib{
	/** Serializing the log
		And the format of log is£º
			output	::= items
			items	::= {item|(indent items)}
			item	::= {content}item_splitter
			content ::= {string|(key keyval_splitter val)}
	*/
	class text_log_serializer
	{
	public:
		text_log_serializer(
			std::_tostream& str,
			const std::_tstring& indent,
			const std::_tstring& item_splitter,
			const std::_tstring& keyval_splitter)		
			:ostr_(str),
			indent_(indent),
			item_splitter_(item_splitter),
			keyval_splitter_(keyval_splitter)
		{
			indent_stack_.push_back(_EFLIB_T(""));
		}

		void push_token_state()
		{
			state_stack_.push_back(boost::make_tuple(indent_, item_splitter_, keyval_splitter_));
		}

		void pop_token_state()
		{
			indent_ = state_stack_.back().get<0>();
			item_splitter_ = state_stack_.back().get<1>();
			keyval_splitter_ = state_stack_.back().get<2>();

			state_stack_.pop_back();
		}

		void set_indent(const std::_tstring indent){
			indent_ = indent;
		}

		void set_item_splitter(const std::_tstring item_splitter)
		{
			item_splitter_ = item_splitter;
		}

		void set_keyval_splitter(const std::_tstring& kvsplitter)
		{
			keyval_splitter_ = kvsplitter;
		}

		~text_log_serializer(){
		}

		void begin_log(){
			indent_stack_.push_back(indent_stack_.back() + indent_);
		}

		void end_log(){
			indent_stack_.pop_back();
		}

		template <class T> void write(const std::_tstring& key, const T& val){
			ostr_ << indent_stack_.back() << key << keyval_splitter_ << val << item_splitter_;
		}

		template <class T> void write(const T& val){
			ostr_ << indent_stack_.back() << val << item_splitter_;
		}

	private:
		text_log_serializer& operator = (const text_log_serializer& rhs);
		text_log_serializer(const text_log_serializer&);

		std::_tstring indent_;
		std::_tstring item_splitter_;
		std::_tstring keyval_splitter_;

		std::vector<boost::tuple<std::_tstring, std::_tstring, std::_tstring> > state_stack_;

		std::vector<std::_tstring> indent_stack_;

		std::_tostream& ostr_;
	};
}
#endif // log_serializer_h__