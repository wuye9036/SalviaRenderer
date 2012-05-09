#include <salviax/include/resource/mesh/sa/material.h>

BEGIN_NS_SALVIAX_RESOURCE();
obj_material::obj_material()
	: name("default")
	, ambient( 0.2f, 0.2f, 0.2f, 1.0f )
	, diffuse( 0.5f, 0.5f, 0.5f, 1.0f )
	, specular( 0.7f, 0.7f, 0.7f, 1.0f )
	, shininess(2), alpha(1.0f)
	, is_specular(true)
	, tex_name("")
{
}
END_NS_SALVIAX_RESOURCE();