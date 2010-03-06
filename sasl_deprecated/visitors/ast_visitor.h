#include "../utility/declare_handle.h"

class ast_node;
class ast_visitor{
	public:
		virtual void enter_node( WEAK_HANDLE_OF( ast_node ) node ) = 0;
		virtual void leave_node( WEAK_HANDLE_OF( ast_node ) node ) = 0;
};