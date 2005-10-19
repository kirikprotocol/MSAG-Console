#ident "$Id$"

#ifndef __SMSC_INMAN_COMMON_SYNC__
#define __SMSC_INMAN_COMMON_SYNC__

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;


namespace smsc  {
namespace inman {
namespace common  {

class Synch : public Mutex
{
    protected:
        Synch() : Mutex()
        {
            Lock();
        }

    public:
        static Synch* getInstance()
        {
            static Synch instance;
            return &instance;
        }
};

} //common
} //inman
} //smsc

#endif /* __SMSC_INMAN_COMMON_SYNC__ */

