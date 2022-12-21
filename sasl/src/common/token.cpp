#include <sasl/common/token.h>

using std::make_shared;
using std::shared_ptr;
using std::string_view;

namespace sasl::common {
void _compile_token() {
  token tok = token::null();
  token tok2 = token::null();
  tok2 = tok;
}
} // namespace sasl::common