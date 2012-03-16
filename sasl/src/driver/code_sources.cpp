class compiler_code_source: public lex_context, public sasl::common::code_source{

private:
	typedef boost::wave::cpplexer::lex_iterator<
		boost::wave::cpplexer::lex_token<> >
		wlex_iterator_t;
	typedef boost::wave::context<
		string::iterator, wlex_iterator_t>
		wcontext_t;

public:
	bool process_code( string const& code, diag_chat* diags ){
		this->code = code;
		this->filename = "in_memory";
		this->diags = diags;
		return process();
	}

	bool process_file( string const& file_name, diag_chat* diags ){
		this->diags = diags;

		std::ifstream in(file_name.c_str(), std::ios_base::in);
		if (!in){
			return false;
		} else {
			in.unsetf(std::ios::skipws);
			std::copy(
				std::istream_iterator<char>(in), std::istream_iterator<char>(),
				std::back_inserter(code) );
			filename = file_name;
		}
		in.close();

		return process();
	}

	// code source
	virtual bool eof(){
		return next_it == wctxt->end();
	}
	virtual string next(){
		assert( next_it != wctxt->end() );
		cur_it = next_it;

		try{
			++next_it;
		} catch ( boost::wave::preprocess_exception& e ){
			errtok = to_std_string( cur_it->get_value() );
			next_it = wctxt->end();
			diags->report( sasl::parser::unknown_tokenize_error ) % e.description();
			cout << e.description() << endl;
		}

		return to_std_string( cur_it->get_value() ) ;
	}
	virtual string error(){
		if( errtok.empty() ){
			errtok = to_std_string( cur_it->get_value() );
		}
		return errtok;
	}

	// lex_context
	virtual const std::string& file_name() const{
		assert( cur_it != wctxt->end() );

		filename = to_std_string( cur_it->get_position().get_file() );
		return filename;
	}
	virtual size_t column() const{
		assert( cur_it != wctxt->end() );
		return cur_it->get_position().get_column();
	}
	virtual size_t line() const{
		assert( cur_it != wctxt->end() );
		return cur_it->get_position().get_line();
	}
	virtual void update_position( const std::string& /*lit*/ ){
		// Do nothing.
		return;
	}

private:
	bool process(){
		wctxt.reset( new wcontext_t( code.begin(), code.end(), filename.c_str() ) );

		size_t lang_flag = wctxt->get_language();
		lang_flag &= ~(boost::wave::support_option_emit_line_directives );
		lang_flag &= ~(boost::wave::support_option_single_line );
		lang_flag &= ~(boost::wave::support_option_emit_pragma_directives );
		wctxt->set_language( static_cast<boost::wave::language_support>( lang_flag ) );

		cur_it = wctxt->begin();
		next_it = wctxt->begin();

		return true;
	}

	template<typename StringT>
	std::string to_std_string( StringT const& str ) const{
		return std::string( str.begin(), str.end() );
	}

	scoped_ptr<wcontext_t>	wctxt;
	diag_chat*				diags;

	string			code;
	string			errtok;
	mutable string	filename;

	wcontext_t::iterator_type cur_it;
	wcontext_t::iterator_type next_it;
};