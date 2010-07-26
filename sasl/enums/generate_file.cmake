macro( sasl_generate_enum output_list_file proj_file depend_files )
ADD_CUSTOM_COMMAND(
	OUTPUT ${output_list_file}
	COMMAND python
	ARGS "${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py" "${proj_file}"
	DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py" ${proj_file} ${depend_files}
)
endmacro( sasl_generate_enum )