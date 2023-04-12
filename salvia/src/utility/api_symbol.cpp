#include <FreeImage.h>
#include <salvia/utility/api_symbols.h>

namespace salvia::utility {
void initialize() {
  FreeImage_Initialise();
}

void finalize() {
  FreeImage_DeInitialise();
}
}  // namespace salvia::utility