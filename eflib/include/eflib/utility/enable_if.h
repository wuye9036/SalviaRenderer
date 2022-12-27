#ifndef EFLIB_METAPROG_ENABLE_IF_H
#define EFLIB_METAPROG_ENABLE_IF_H

#include <boost/preprocessor/cat.hpp>
#include <type_traits>

#define EFLIB_ENABLE_IF_COND(cond) typename ::std::enable_if<cond ::value>::type * = nullptr

#define EFLIB_DISABLE_IF_COND(cond) typename ::std::enable_if<!(cond ::value)>::type * = nullptr

#define EFLIB_ENABLE_IF_PRED1(pred, T)                                                             \
  typename ::std::enable_if<std::pred<T>::value>::type * = nullptr

#define EFLIB_DISABLE_IF_PRED1(pred, T)                                                            \
  typename ::std::enable_if<std::pred<T>::value>::type * = nullptr

#define EFLIB_ENABLE_IF_PRED2(pred, U, T)                                                          \
  typename ::std::enable_if<std::pred<U, T>::value>::type * = nullptr

#define EFLIB_DISABLE_IF_PRED2(pred, U, T)                                                         \
  typename ::std::enable_if<!std::pred<U, T>::value>::type * = nullptr

#endif
