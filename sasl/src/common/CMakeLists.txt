set( HEADER_LIST
	../../include/sasl/common/common_fwd.h
	../../include/sasl/common/diag_chat.h
	../../include/sasl/common/diag_item.h
	../../include/sasl/common/diag_formatter.h
	../../include/sasl/common/lex_context.h
	../../include/sasl/common/token.h
)

set( SOURCE_LIST diag_chat.cpp token.cpp diag_formatter.cpp)

add_library(sasl_common ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_common PUBLIC ../../include)
target_link_libraries(sasl_common eflib fmt::fmt)
target_compile_features(sasl_common PUBLIC cxx_std_20)

