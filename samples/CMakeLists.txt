option( SALVIA_ENABLE_SAMPLES "Build samples." TRUE )

function(configure_sample_project project_name source_list dependencies)
	add_executable(${project_name} ${source_list})
	target_link_libraries(${project_name}
			salvia_utility salvia_ext salvia_core salvia_shader salvia_resource
			eflib ${dependencies})
	set_target_properties(${project_name} PROPERTIES FOLDER "Samples")
endfunction()

add_subdirectory(ComplexMesh)

#	ADD_SUBDIRECTORY( TextureAndBlending )
#	ADD_SUBDIRECTORY( ColorizedTriangle )
#	ADD_SUBDIRECTORY( ObjLoader )
#	ADD_SUBDIRECTORY( Sponza )
#	ADD_SUBDIRECTORY( PartOfSponza )
#	ADD_SUBDIRECTORY( Triangles )
#	ADD_SUBDIRECTORY( Morph )
#	ADD_SUBDIRECTORY( VertexTextureFetch )
#	ADD_SUBDIRECTORY( AnisotropicFilter )
#	ADD_SUBDIRECTORY( Font )
#	ADD_SUBDIRECTORY( StandardShadowMap )
#	ADD_SUBDIRECTORY( StencilMirror )
#	ADD_SUBDIRECTORY( AntiAliasing )

# Copy resources required by sample to target folder