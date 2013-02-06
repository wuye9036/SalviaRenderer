#include <salviar/include/binary_modules.h>

#include <eflib/include/platform/dl_loader.h>

using eflib::dynamic_lib;

using boost::shared_ptr;

using std::string;
using std::vector;

BEGIN_NS_SALVIAR();

namespace modules
{
	static void (*compile_func)(
		shader_object_ptr&, shader_log_ptr&,
		string const&, shader_profile const&,
		vector<external_function_desc> const&
		) = NULL;
	static void (*create_host_func)(host_ptr& out) = NULL;
	
	static shared_ptr<dynamic_lib> host_lib;
	
	static void load_function()
	{
		assert(!host_lib);
		std::string dll_name = "sasl_host";
#ifdef EFLIB_DEBUG
		dll_name += "_d";
#endif
		dll_name += ".dll";

		host_lib = dynamic_lib::load(dll_name);
		host_lib->get_function(compile_func, 	"salvia_compile_shader");
		host_lib->get_function(create_host_func,"salvia_create_host");
		assert(compile_func);
		assert(create_host_func);
	}
	
	void host::compile(
		shader_object_ptr& obj, shader_log_ptr& log,
		string const& code, shader_profile const& prof,
		vector<external_function_desc> const& funcs )
	{
		if(!compile_func)
		{
			load_function();
		}
		assert(compile_func);

		if(!compile_func) return;
		compile_func(obj, log, code, prof, funcs);
	}
	
	host_ptr host::create_host()
	{
		host_ptr ret;
		
		if(!create_host_func)
		{
			load_function();
		}
		assert(create_host_func);
		
		if (create_host_func)
		{
			create_host_func(ret);
		}
		
		return ret;
	}
}
END_NS_SALVIAR();