
#ifndef SMSC_MSCMAN_MSC_ADMIN
#define SMSC_MSCMAN_MSC_ADMIN

#include <string>

#include <core/buffers/Array.hpp>

#include "MscExceptions.h"

namespace smsc { namespace mscman
{
    using std::string;
    using smsc::core::buffers::Array;

    struct MscInfo
    {
        MscInfo(const char* msc="", bool mLock=false, bool aLock=false, int fc=0)
            : manualLock(mLock),
                automaticLock(aLock), failureCount(fc)
            {
              setMscNum(msc);
            };
        MscInfo(const MscInfo& info)
            : manualLock(info.manualLock),
                automaticLock(info.automaticLock),
                    failureCount(info.failureCount)
            {
              setMscNum(info.mscNum);
            };

        MscInfo& operator=(const MscInfo& info) {
            setMscNum(info.mscNum);
            manualLock = info.manualLock;
            automaticLock = info.automaticLock;
            failureCount = info.failureCount;
            return (*this);
        }

        void setMscNum(const char* msc)
        {
          strncpy(mscNum,msc,sizeof(mscNum)-1);
          mscNum[sizeof(mscNum)-1]=0;
        }

        char    mscNum[22];
        bool    manualLock;
        bool    automaticLock;
        int     failureCount;
    };

    class MscAdmin
    {
    protected:

        MscAdmin() {};

    public:

        virtual ~MscAdmin() {};

        virtual void registrate(const char* msc) = 0;
        virtual void unregister(const char* msc) = 0;
        virtual void block(const char* msc) = 0;
        virtual void clear(const char* msc) = 0;
        virtual Array<MscInfo> list() = 0;
    };
}}

#endif //SMSC_MSCMAN_MSC_ADMIN
