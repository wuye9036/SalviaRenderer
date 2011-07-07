#include <tchar.h>

#include <salviau/include/wtl/wtl_application.h>

using namespace salviau;

int main( int argc, TCHAR* argv[] ){

	application* papp = create_wtl_application();
	if( !papp ){ return -1; }
	papp->run();
	delete papp;

	return 0;
}