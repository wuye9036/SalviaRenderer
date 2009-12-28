#ifndef SASL_PARSER_ADAPT_INSTRUSIVE_STRUCT_HANDLE_H
#define SASL_PARSER_ADAPT_INSTRUSIVE_STRUCT_HANDLE_H

#include <boost/fusion/support/tag_of_fwd.hpp>
#include <boost/fusion/adapted/struct/extension.hpp>
#include <boost/fusion/adapted/struct/struct_iterator.hpp>
#include <boost/fusion/adapted/struct/detail/is_view_impl.hpp>
#include <boost/fusion/adapted/struct/detail/is_sequence_impl.hpp>
#include <boost/fusion/adapted/struct/detail/category_of_impl.hpp>
#include <boost/fusion/adapted/struct/detail/begin_impl.hpp>
#include <boost/fusion/adapted/struct/detail/end_impl.hpp>
#include <boost/fusion/adapted/struct/detail/size_impl.hpp>
#include <boost/fusion/adapted/struct/detail/at_impl.hpp>
#include <boost/fusion/adapted/struct/detail/value_at_impl.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/mpl/int.hpp>
#include <boost/config/no_tr1/utility.hpp>

#define SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE(name, bseq)                                   \
    SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_I(                                                \
        name, BOOST_PP_CAT(SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_X bseq, 0))                \
    /***/

#define SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_X(x, y) ((x, y)) SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_Y
#define SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_Y(x, y) ((x, y)) SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_X
#define SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_X0
#define SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_Y0

// SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_I generates the overarching structure and uses
// SEQ_FOR_EACH_I to generate the "linear" substructures.
// Thanks to Paul Mensonides for the PP macro help

#define SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_I(name, seq)                                  \
    namespace boost { namespace fusion { namespace traits                       \
    {                                                                           \
        template <>                                                             \
        struct tag_of<name>                                                     \
        {                                                                       \
            typedef struct_tag type;                                            \
        };                                                                      \
    }}}                                                                         \
    namespace boost { namespace fusion { namespace extension                    \
    {                                                                           \
        template <>                                                             \
        struct struct_size<name> : mpl::int_<BOOST_PP_SEQ_SIZE(seq)> {};        \
        BOOST_PP_SEQ_FOR_EACH_I(SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_C, name, seq)         \
    }}}                                                                         \
    /***/

#define SASL_ADAPT_INSTRUSIVE_STRUCT_HANDLE_C(r, name, i, xy)                             \
    template <>                                                                 \
    struct struct_member<name, i>                                               \
    {                                                                           \
        typedef BOOST_PP_TUPLE_ELEM(2, 0, xy) type;                             \
        static type& call(name& handle_)                                        \
        {                                                                       \
            return handle_->BOOST_PP_TUPLE_ELEM(2, 1, xy);                       \
        };                                                                      \
    };                                                                          \
    /***/

#endif // SASL_PARSER_ADAPT_INSTRUSIVE_STRUCT_HANDLE_H