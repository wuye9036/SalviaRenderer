#ifndef EFLIB_STRING_USTRING_H
#define EFLIB_STRING_USTRING_H

template <typename CharT>
class basic_ustring{
	basic_ustring( std::string )
	std::basic_string<CharT> content;
	
	intptr_t volatile ref_count;
};

#endif