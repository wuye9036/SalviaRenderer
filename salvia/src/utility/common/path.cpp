#include <salvia/utility/common/path.h>

#include <filesystem>

using std::vector;

using std::string;
using std::wstring;

using std::filesystem::path;

namespace salvia::utility {

std::vector<std::string> default_search_paths() {
  return {"./resources", "../resources", "../../resources", "../../../resources"};
}

template <typename StringT>
StringT find_path_impl(StringT const &relative_path, vector<StringT> const &candidates) {
  for (auto const &parent_folder : candidates) {
    auto full_path = path(parent_folder) / path(relative_path);
    if (std::filesystem::exists(full_path)) {
      return full_path.string<typename StringT::value_type>();
    }
  }
  return StringT();
}

string find_path(string const &relative_path) {
  return find_path_impl(relative_path, default_search_paths());
}

string find_path(string const &relative_path, vector<string> const &candidates) {
  return find_path_impl(relative_path, candidates);
}

} // namespace salvia::utility
