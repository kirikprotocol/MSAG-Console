#ifndef _SCAG_UTIL_THREADWRAP_H
#define _SCAG_UTIL_THREADWRAP_H

#include "core/threads/Thread.hpp"

namespace scag {
namespace util {

    /// wrapper for thread
    template < class T > class ThreadWrap : public smsc::core::threads::Thread
    {
    protected:
        typedef int (T::*fun_t)( void );

    public:
        ThreadWrap( T& inst, fun_t fun ) : inst_(&inst), fun_(fun) {}

        virtual int Execute() {
            return (inst_->*fun_)();
        }

        virtual ~ThreadWrap() {}

    private:
        T*     inst_;
        fun_t  fun_;
    };

} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_THREADWRAP_H */
