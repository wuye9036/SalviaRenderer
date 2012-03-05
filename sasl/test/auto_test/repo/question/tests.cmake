set( SALVIA_AUTO_TEST_HOME "${SASL_HOME_DIR}/sasl/test/auto_test/repo/question" )

set( V1A1_TESTS
	"${SALVIA_AUTO_TEST_HOME}/v1a1/comments.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/constructors.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/constructors.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/ddx_ddy.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/decl.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/deps.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/null.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/preprocessors.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/semantic.svs"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/semantic_fn.svs"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/semfn_par.svs"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/struct_semin.svs"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/vec_and_mat.svs"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/vec_and_mat.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/arithmetic.svs"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/arithmetic.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/function.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/intrinsics.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/intrinsics.svs"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/intrinsics.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/branches.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/branches.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/empty.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/bool.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/unary_operators.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/host_intrinsic_detection.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/initializer.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/casts.ss"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/scalar.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/swizzle_and_wm.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/tex.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/for_loop.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/while.sps"
	"${SALVIA_AUTO_TEST_HOME}/v1a1/do_while.sps"
)

SOURCE_GROUP( "Tests\\v1a1\\General" REGULAR_EXPRESSION ".*\\.ss" )
SOURCE_GROUP( "Tests\\v1a1\\Vertex Shaders" REGULAR_EXPRESSION ".*\\.svs" )
SOURCE_GROUP( "Tests\\v1a1\\Pixel Shaders" REGULAR_EXPRESSION ".*\\.sps" )