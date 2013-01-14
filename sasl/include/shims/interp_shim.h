class interp_shim
{
public:
	void* get_shim_function(
		salviar::shader_reflection_ptr const&	vs_reflection,
		salviar::shader_reflection_ptr const&	ps_reflection
	);
};