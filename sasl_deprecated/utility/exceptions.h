class no_enum_exception: public std::exception{
	no_enum_exception( const std::string& desc ): exp_desc( desc ){
	}
	
	const char* what() const{
		return exc_desc.c_str();
	}

private:
	std::string exc_desc;
};

class key_not_found_exception: public std::exception{
	key_not_found_exception( const std::string& desc = std::string() )
		:exc_desc(desc)
	{
	}
	
	const char* what() const{
		return exc_desc.c_str();
	}
	
private:
	std::string exc_desc;
};

class key_exists_exception: public std::exception{
	key_exists_exception( const std::string& desc = std::string() )
		:exc_desc(desc)
	{
	}
	
	const char* what() const{
		return exc_desc.c_str();
	}
	
private:
	std::string exc_desc;
};