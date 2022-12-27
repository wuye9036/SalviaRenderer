#include <sasl/common/token.h>

using std::make_shared;
using std::shared_ptr;
using std::string_view;

namespace sasl::common {
void _compile_token() {
  token tok = token::make_empty();
  token tok2 = token::make_empty();
  tok2 = tok;
}
} // namespace sasl::common