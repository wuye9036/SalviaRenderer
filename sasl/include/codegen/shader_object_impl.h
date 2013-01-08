
class shader_impl: public shader_object
{
public:
	
private:
	shader_reflection_ptr						reflection_;
	cg_module_ptr 								module_;
	boost::shared_ptr<llvm::ExecutionEngine>	engine_;
	std::vector<llvm::Function*>				vm_functions_;
	eflib::fixed_string							vm_err_;
};