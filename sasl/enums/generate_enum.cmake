function( sasl_generate_enum output_list_file proj_file depend_files )

LIST(APPEND depends_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/${proj_file}")

foreach( depend_item ${depend_files} )
LIST( APPEND depends_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/${depend_item}" )
endforeach( depend_item )

ADD_CUSTOM_COMMAND(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${output_list_file}"
	COMMAND cd
	ARGS \"${CMAKE_CURRENT_SOURCE_DIR}\"
	COMMAND python
	ARGS "${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py" "${proj_file}"
	DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/${proj_file}" ${depends_fullpath}
)

MESSAGE( "${output_list_file} depends following files:" )
foreach( depend_item ${depends_fullpath} )
MESSAGE( " - ${depend_item}" )
endforeach( depend_item )

endfunction( sasl_generate_enum )