find_package(llvm CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs
  Analysis Core ExecutionEngine
  InstCombine Object OrcJIT RuntimeDyld ScalarOpts
  Support native MCJIT X86CodeGen
)