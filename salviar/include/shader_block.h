#ifndef SALVIA_SHADER_BLOCK_H
#define SALVIA_SHADER_BLOCK_H

#include <softart/include/shader_enums.h>

#include <sasl/include/semantic/abi_info.h>

BEGIN_NS_SALVIA();

class vertex_shader_block
{
public:
	size_t buffer_size( sasl::semantic::storage_type );
	size_t offset( softart::semantic );
	size_t variable_size( sasl::semantic::storage_type, softart::semantic );
	builtin_type_code variable_type();
private:
	void* code;
};

END_NS_SALVIA();

#endif