#include <eflib/include/memory/lifetime_manager.h>

namespace eflib{

	lifetime_manager* lifetime_manager::inst = NULL;

	lifetime_manager::lifetime_manager(){
		assert( !inst );
		inst = this;
	}

	lifetime_manager::~lifetime_manager(){
		assert( inst );
		if ( inst ){
			inst = NULL;
			for( std::vector< boost::function<void()> >::reverse_iterator it = exit_callbacks.rbegin();
				it != exit_callbacks.rend(); ++it )
			{
				// invoke. FILO.
				(*it)();
			}
		}
	}

	void lifetime_manager::at_main_exit( boost::function<void()> exit_func ){
		assert( inst && "Error: Lifetime Manager was not initialized or released yet." );
		inst->exit_callbacks.push_back( exit_func );
	}

}