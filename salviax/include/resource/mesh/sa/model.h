#ifndef SALVIAX_RESOURCE_MODEL_H
#define SALVIAX_RESOURCE_MODEL_H

#include <slaviax/include/resource/resource_forward.h>

BEGIN_NS_SALVIAX_RESOURCE();

class model{
public:
	void add_mesh( salviar::h_mesh const& );
	
	void render();
	
private:
	std::vector<h_mesh> meshes;
};

END_NS_SALVIAX_RESOURCE();

#endif