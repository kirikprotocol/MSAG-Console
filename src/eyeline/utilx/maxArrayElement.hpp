#ifndef __EYELINE_UTILX_MAXARRAYELEMENT_HPP__
# define __EYELINE_UTILX_MAXARRAYELEMENT_HPP__

#include <algorithm>

namespace eyeline {
namespace utilx {

template <class T,size_t N>
const T& maxArrayElement(const T (&arr)[N])
{
 return *std::max_element(arr,arr+N);
}

}}

#endif
