#ifndef SALVIA_SHADER_UNIT_H
#define SALVIA_SHADER_UNIT_H

BEGIN_NS_SALVIA();

class shader_code;

class vertex_shader_unit
{
	void initialize( shader_code const* );
	void update();
	void execute();
};

END_NS_SALVIA();

#endif