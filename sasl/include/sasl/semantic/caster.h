#ifndef SASL_SEMANTIC_TYPE_CONVERTER_H
#define SASL_SEMANTIC_TYPE_CONVERTER_H

#include <sasl/semantic/semantic_forward.h>

#include <sasl/semantic/pety.h>

#include <boost/bimap.hpp>
#include <boost/multi_index_container.hpp>

#include <eflib/utility/hash.h>

#include <functional>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace sasl {
namespace syntax_tree {
struct node;
}
} // namespace sasl

namespace sasl::semantic {

namespace sst = sasl::syntax_tree;

class node_semantic;

using get_tynode_fn = std::function<sst::tynode *(tid_t)>;
using get_semantic_fn = std::function<node_semantic *(sst::node *)>;

class caster_t {
public:
  enum casts { eql = 0, imp, exp, nocast = 0xFFFFFFFF };

  using cast_t = std::function<void(sst::node *, sst::node *)>;

  caster_t();

  void add_cast(casts ct, tid_t src, tid_t dest, cast_t conv);
  void add_cast(casts ct, int prior, tid_t src, tid_t dest, cast_t conv);
  void add_cast_auto_prior(casts ct, tid_t src, tid_t dest, cast_t conv);

  casts try_cast(int &prior, tid_t dest, tid_t src);
  casts try_cast(tid_t dest, tid_t src);
  bool try_implicit(tid_t dest, tid_t src);

  void better_or_worse(tid_t matched, tid_t matching, tid_t src, bool &better, bool &worse);

  casts cast(std::shared_ptr<sst::node> dest, std::shared_ptr<sst::node> src);
  casts cast(sst::node *dest, sst::node *src);

  void set_function_get_tynode(get_tynode_fn fn);
  void set_function_get_semantic(get_semantic_fn fn);

  virtual ~caster_t() {}

private:
  typedef std::tuple<casts /*result*/, int /*prior*/, tid_t /*src*/, tid_t /*dest*/,
                     cast_t /*caster*/
                     >
      cast_info;

  typedef std::unordered_map<std::pair<tid_t /*src*/, tid_t /*dest*/>, size_t /*cast info index*/, eflib::hash_tuple
                             >
      cast_info_dict_t;

  cast_info const *find_caster(cast_info const *&first_caster, cast_info const *&second_caster,
                               tid_t &immediate_tid, tid_t dest, tid_t src,
                               bool direct_caster_only); // return non-equal caster.

  std::unordered_map<tid_t, int> lowest_priors; // For auto cast priority.
  std::vector<cast_info> cast_infos;
  cast_info_dict_t cast_info_dict;
  boost::bimap<tid_t, tid_t> eql_casts;

  // Functions
  get_tynode_fn get_tynode_;
  get_semantic_fn get_semantic_;
};

} // namespace sasl::semantic

#endif
