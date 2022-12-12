set( HEADER_LIST
	../../include/sasl/parser/diags.h
	../../include/sasl/parser/error_handlers.h
	../../include/sasl/parser/lexer.h
	../../include/sasl/parser/generator.h
	../../include/sasl/parser/grammars.h
	../../include/sasl/parser/parse_api.h
	../../include/sasl/parser/parser_forward.h
)

set( SOURCE_LIST
	diags.cpp
	error_handlers.cpp
	generator.cpp
	grammars.cpp
	lexer.cpp
	parse_api.cpp
)

add_library(sasl_parser ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_parser PUBLIC ../../include)
target_link_libraries(sasl_parser eflib)
target_compile_features(sasl_parser PUBLIC cxx_std_20)

set_target_properties(sasl_parser PROPERTIES FOLDER "Shader Libs")

