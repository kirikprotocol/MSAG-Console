#ifndef _SCAG_UTIL_XHASHPTRFUNC_H
#define _SCAG_UTIL_XHASHPTRFUNC_H

namespace scag2 {
namespace util {

struct XHashPtrFunc
{
    template <typename T> static inline unsigned int CalcHash(const T* const key)
    {
        return unsigned(reinterpret_cast<unsigned long long>(static_cast<const void*>(key)));
    }
};

} // namespace util
} // namespace scag2


namespace scag {
namespace util {
using scag2::util::XHashPtrFunc;
}
}

#endif /* !_SCAG_UTIL_XHASHPTRFUNC_H */
