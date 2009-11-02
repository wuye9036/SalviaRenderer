#ifndef EFLIB_STREAMX_H
#define EFLIB_STREAMX_H

#include "../config.h"

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
	typedef cin _tcin;
	typedef cout _tcout;
	typedef ostream _tostream;
	typedef istream _tistream;
	typedef fstream _tfstream;
	typedef stringstream _tstringstream;
#endif
}

#endif //EFLIB_STREAMX_H