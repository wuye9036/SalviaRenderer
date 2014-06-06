enum class reg_slot: uint32_t
{
	unknown = 0,
	uniforms,
	mutables,
	outputs,
	count
};

enum class reg_type
{
	none = 0,
	texture,
	sampler,
	uav,
	cbuffer,
	output,
	count
};

struct reg_name
{
	uint32_t	buffer_index;
	uint32_t	reg_index;
	uint32_t	elem_index;
	reg_type	type;
};

typedef size_t register_handle;
class rfile_sasl_mapping
{
	void			initialize(shader_profile prof);
	
	register_handle alloc_reg(data_type dtype, semantic sem, reg_name const& rname);
	
	void 			update();
	
	size_t			register_count(reg_slot slot);
	void			physical_pos(reg_slot& slot, size_t& elem_offset, register_handle handle);
	
private:
	struct reg_info
	{
		data_type 	type;
		semantic	sem;
		reg_name	rname;
	};
	
	std::vector<
		std::pair<reg_slot, size_t>
		>				  reg_pos_;
	std::vector<reg_info> reg_infos_;
};

class rfile_hlsl_bc_mapping
{
	
};