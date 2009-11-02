typename 
class compiler_error{
public:
	compiler_error( compiler_errors err_code, const string& desc, const token_location<GlobalType::iterator_t>& loc );
};