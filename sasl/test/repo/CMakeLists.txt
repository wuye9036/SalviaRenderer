SALVIA_CHECK_BUILD_WITH_UNICODE()

set( TEST_SHADERS
	"auto_semantic.svs"
	"array.svs"
	"array_and_index.ss"
	"assigns.ss"
	"comments.ss"
	"constructors.ss"
	"constructors.sps"
	"ddx_ddy.sps"
	"decl.ss"
	"deps.ss"
	"null.ss"
	"preprocessors.ss"
	"semantic.svs"
	"semantic_fn.svs"
	"semfn_par.svs"
	"struct_semin.svs"
	"vec_and_mat.svs"
	"vec_and_mat.sps"
	"arithmetic.ss"
	"arithmetic.sps"
	"function.ss"
	"input_assigned.svs"
	"intrinsics.ss"
	"intrinsics.svs"
	"intrinsics.sps"
	"branches.ss"
	"branches.sps"
	"empty.ss"
	"bool.ss"
	"unary_operators.ss"
	"host_intrinsic_detection.ss"
	"initializer.ss"
	"casts.ss"
	"scalar.sps"
	"swizzle.ss"
	"swizzle_and_wm.sps"
	"tex.svs"
	"tex.sps"
	"for_loop.sps"
	"while.sps"
	"do_while.sps"
	"local_var.ss"
	"bit_ops.ss"
	
	"include_header.ss"
	"include_main.ss"
	"include_search_path.ss"
	"incomplete.ss"
	"semantic_errors.ss"
	
	"include/header.ss"
	"sysincl/sysheader.ss"
)

SOURCE_GROUP( "Shaders\\General" 		REGULAR_EXPRESSION ".*\\.ss"  )
SOURCE_GROUP( "Shaders\\Vertex Shaders" REGULAR_EXPRESSION ".*\\.svs" )
SOURCE_GROUP( "Shaders\\Pixel Shaders"	REGULAR_EXPRESSION ".*\\.sps" )

if (MSVC)
	SET( SALVIA_TEST_REPOSITORY_PATH "${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/repo" )
else( MSVC )
	SET( SALVIA_TEST_REPOSITORY_PATH "${SALVIA_RUNTIME_OUTPUT_DIRECTORY}/repo" )
endif()

ADD_CUSTOM_COMMAND(
		OUTPUT	test_repo.deps
		COMMAND "${PYTHON_EXECUTABLE}"
		ARGS
			"${SASL_HOME_DIR}/sasl/test/repo/copy_tests.py"
			"${SASL_HOME_DIR}/sasl/test/repo"
			${SALVIA_TEST_REPOSITORY_PATH}
		COMMAND echo "test_repo.deps" > test_repo.deps
		DEPENDS
			"${SASL_HOME_DIR}/sasl/test/repo/copy_tests.py" ${TEST_SHADERS}
		)
ADD_CUSTOM_TARGET( sasl_test_shader_repo ALL DEPENDS test_repo.deps ${TEST_SHADERS} SOURCES ${TEST_SHADERS} )
SET_TARGET_PROPERTIES( sasl_test_shader_repo PROPERTIES FOLDER "Shader Tests")