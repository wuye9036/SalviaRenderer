BEGIN_NS_SASL_COMPILER();

template <typename ParserT>
bool compiler::parse( ParserT& parser )
{
	try{
		opt_disp.reg_extra_parser( parser );
		opt_global.reg_extra_parser( parser );
		opt_io.reg_extra_parser( parser );
		opt_predef.reg_extra_parser( parser );

		po::parsed_options parsed
			= parser.run();

		std::vector<std::string> unrecg = po::collect_unrecognized( parsed.options, po::include_positional );

		if( !unrecg.empty() ){
			cout << "Warning: options ";
			BOOST_FOREACH( std::string const & str, unrecg ){
				cout << str << " ";
			}
			cout << "are invalid. They were ignored." << endl;
		}

		po::store( parsed, vm );
		po::notify(vm);

	} catch ( boost::program_options::invalid_command_line_syntax const & e ) {
		cout << "Fatal error occurs: " << e.what() << endl;
		return false;
	} catch ( std::exception const & e ){
		cout << "Unprocessed error: " << e.what() << endl;
	}

	return true;
}

bool compiler::parse( int argc, char** argv )
{
	po::basic_command_line_parser<char> parser
		= po::command_line_parser(argc, argv).options( desc ).allow_unregistered();
	return parse(parser);
}

bool compiler::parse( std::string const& cmd )
{
	vector<string> cmds = po::split_unix(cmd);
	po::basic_command_line_parser<char> parser
		= po::command_line_parser(cmds).options( desc ).allow_unregistered();

	return parse( parser );
}

compiler::compiler()
{
	opt_disp.fill_desc(desc);
	opt_global.fill_desc(desc);
	opt_io.fill_desc(desc);
	opt_predef.fill_desc(desc);
}

void compiler::process( bool& abort )
{
	abort = false;

	opt_disp.filterate(vm);
	opt_global.filterate(vm);
	opt_io.filterate(vm);
	opt_predef.filterate(vm);

	if( opt_disp.help_enabled() ){
		cout << desc << endl;
		return;
	}

	if( opt_disp.version_enabled() ){
		cout << opt_disp.version() << endl;
		return;
	}

	if( opt_global.detail() == options_global::none ){
		cout << "Detail level is an invalid value. Ignore it." << endl;
	}

	// Process inputs and outputs.
	vector<string> inputs = opt_io.inputs();

	if( inputs.empty() ){
		cout << "Need at least one input file." << endl;
		return;
	}

	// TODO
	salviar::languages lang = opt_io.language();

	EFLIB_ASSERT_AND_IF( lang != salviar::lang_none, "Can not support language guessing by file extension yet." ){
		return;
	}

	if( opt_io.format() == options_io::llvm_ir ){
		BOOST_FOREACH( string const & fname, inputs ){
			cout << "Compile " << fname << "..." << endl;

			shared_ptr<diag_chat> diags = diag_chat::create();

			shared_ptr<compiler_code_source> code_src( new compiler_code_source() );
			if ( !code_src->process_file(fname, diags.get()) ){
				diags->report( sasl::parser::cannot_open_input_file ) % fname;
				return;
			} 

			mroot = sasl::syntax_tree::parse( code_src.get(), code_src, diags.get() );
			if( !mroot ){
				cout << "Syntax error occurs!" << endl;
				abort = true;
				return;
			}

			msi = analysis_semantic( mroot );
			if( !msi ){
				cout << "Semantic error occurs!" << endl;
				abort = true;
				return;
			}

			abi_analyser aa;

			if( !aa.auto_entry( msi, lang ) ){
				if ( lang != salviar::lang_general ){
					cout << "ABI analysis error occurs!" << endl;
					abort = true;
					return;
				}
			}

			shared_ptr<llvm_module> llvmcode = generate_llvm_code( msi.get(), aa.abii(lang) );
			mcg = llvmcode;

			if( !llvmcode ){
				cout << "Code generation error occurs!" << endl;
				abort = true;
				return;
			}

			if( !opt_io.output().empty() ){
				ofstream out_file( opt_io.output().c_str(), std::ios_base::out );
				dump( llvmcode, out_file );
			}

		}
	}
}

shared_ptr< module_si > compiler::module_sem() const{
	return msi;
}

shared_ptr<codegen_context> compiler::module_codegen() const{
	return mcg;
}

shared_ptr<node> compiler::root() const{
	return mroot;
}

po::variables_map const & compiler::variables() const
{
	return vm;
}

options_display_info const & compiler::display_info() const
{
	return opt_disp;
}

options_io const & compiler::io_info() const
{
	return opt_io;
}
