#ifndef EFLIB_IO_STREAM_H
#define EFLIB_IO_STREAM_H

#include <eflib/include/platform/config.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace std{
#ifdef EFLIB_UNICODE
	static wistream& _tcin(wcin);
	static wostream& _tcout(wcout);
	typedef wostream _tostream;
	typedef wistream _tistream;
	typedef wfstream _tfstream;
	typedef wstringstream _tstringstream;
#else
	static istream& _tcin(cin);
	static ostream& _tcout(cout);
	typedef ostream _tostream;
	typedef istream _tistream;
	typedef fstream _tfstream;
	typedef stringstream _tstringstream;
#endif
}

#endif //EFLIB_STREAMX_H