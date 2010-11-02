#ifndef EFLIB_MEMORY_LIFETIME_MANAGER_H
#define EFLIB_MEMORY_LIFETIME_MANAGER_H

#include <boost/function.hpp>
#include <vector>

namespace eflib{
	// This class will used as following:
	//	int main(){
	//		lifetime_manager lfmgr;
	//		// ...
	//		// other codes
	//		// ...
	//		return 0;
	//	}
	// If following this case, registered function was called after main function exited.
	// We can resource release before static object releasing
	class lifetime_manager{
	public:
		static void at_main_exit( boost::function<void()> exit_func );
		lifetime_manager();
		~lifetime_manager();
	private:
		static lifetime_manager* inst;

		// prevent "new" operator.
		void* operator new( size_t size );
		// storage
		std::vector<boost::function<void()> > exit_callbacks;

	};
}

#endif