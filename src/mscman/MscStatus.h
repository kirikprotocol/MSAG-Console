
#ifndef SMSC_MSCMAN_MSC_STATUS
#define SMSC_MSCMAN_MSC_STATUS

#include "core/buffers/File.hpp"

namespace smsc { namespace mscman
{
  enum MscState{
    mscLocked=0,
    mscUnlocked=1,
    mscUnlockedOnce=2
  };
  class MscStatus
    {
    protected:

        MscStatus() {};

    public:

        virtual ~MscStatus() {};

        virtual void report(const char* msc, bool status) = 0;
        virtual MscState check(const char* msc) = 0;
    };
}}

#endif //SMSC_MSCMAN_MSC_STATUS
