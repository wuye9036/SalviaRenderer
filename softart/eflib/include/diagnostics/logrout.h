#include <string>

namespace eflib{
	namespace logrout{
		std::string screen();
		std::string file();
		
		std::string on();
		std::string off();
		
		std::string state( const std::string& dev, const std::string& s );
		void write_state( const std::string& dev, const std::string& s );

		void fflush_all();
	}
}