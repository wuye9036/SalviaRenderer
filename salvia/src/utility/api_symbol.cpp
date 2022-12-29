#include <salvia/utility/api_symbols.h>
#include <FreeImage.h>

namespace salvia::utility{
void initialize() {
  FreeImage_Initialise();
}

void finalize() {
  FreeImage_DeInitialise();
}
}