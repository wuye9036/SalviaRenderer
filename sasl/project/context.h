#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "../utility/declare_handle.h"

#include <string>
#include <vector>

class symbol_table;
class unit;

DECL_HANDLE( ast_node );

class context
{
public:
	struct code_pos{
		code_pos(const std::string& filename, int line_idx)
			:filename(filename), line_idx(line_idx){}
		std::string filename;
		int	line_idx;
	};

	context(unit* owner);

	const std::string& get_current_line();
	void set_current_line(const std::string& line_str);

	int get_current_line_idx();
	void set_current_line_idx(int line_idx);

	code_pos get_code_pos();
	void set_code_pos(const code_pos& pos);

	std::vector<h_ast_node>& get_node_stack();
private:
	std::string		current_line_;
	int				current_line_idx_;
	std::string 	filename_;
	unit*			current_unit_;
	std::vector<h_ast_node> node_stack_;
};

#endif // CONTEXT_H_INCLUDED
