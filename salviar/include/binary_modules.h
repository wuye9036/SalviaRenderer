#pragma once

#ifndef SALVIAR_BINARY_MODULES_H
#define SALVIAR_BINARY_MODULES_H

#include <eflib/include/platform/typedefs.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>

#include <salviar/include/salviar_forward.h>

BEGIN_NS_SALVIAR();
namespace modules
{
	class host
	{
	public:
		static void compile(
			shader_object_ptr& obj, shader_log_ptr& log,
			string const& code, shader_profile const& prof,
			vector<external_function_desc> const& funcs
			);
		static host_ptr create_host();
	};

}

END_NS_SALVIAR();

#endif