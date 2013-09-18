#pragma once

#ifndef SALVIAR_BINARY_MODULES_H
#define SALVIAR_BINARY_MODULES_H

#include <salviar/include/salviar_forward.h>

#include <eflib/include/utility/shared_declaration.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>
#include <string>

BEGIN_NS_SALVIAR();

EFLIB_DECLARE_CLASS_SHARED_PTR(host);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_log);
EFLIB_DECLARE_CLASS_SHARED_PTR(shader_object);

struct external_function_desc;
struct shader_profile;

namespace modules
{
	class host
	{
	public:
		static void compile(
			shader_object_ptr& obj, shader_log_ptr& log,
			std::string const& code, shader_profile const& prof,
			std::vector<external_function_desc> const& funcs
			);
		static void compile_from_file(
			shader_object_ptr& obj, shader_log_ptr& log,
			std::string const& file_name, shader_profile const& prof,
			std::vector<external_function_desc> const& funcs
			);
		static host_ptr create_host();
	};

}

END_NS_SALVIAR();

#endif