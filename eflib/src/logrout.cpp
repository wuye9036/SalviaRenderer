#include <eflib/include/diagnostics/logrout.h>

#include <stdio.h>

namespace eflib{
	namespace logrout{
		std::string screen(){
			return std::string("$screen$");
		}
		
		std::string logfile(){
			return std::string("$file$");
		}
		
		std::string on(){
			return std::string("$on$");
		}
		
		std::string off(){
			return std::string("$off$");
		}
		
		std::string state( const std::string& dev, const std::string& s ){
			return dev + ":" + s;
		}

		void write_state( const std::string& dev, const std::string& s )
		{
			fputs( state(dev, s).c_str(), stdout );
			fputs( "\r\n", stdout );
			fflush_all();
		}

		void fflush_all(){
			fflush(stderr);
			fflush(stdout);
		}
	}
}