
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
        MscInfo(string msc, bool mLock=false, bool aLock=false, int fc=0)
            : mscNum(msc), manualLock(mLock), 
                automaticLock(aLock), failureCount(fc) {};
        MscInfo(const MscInfo& info)
            : mscNum(info.mscNum), manualLock(info.manualLock),
                automaticLock(info.automaticLock), 
                    failureCount(info.failureCount) {};
        
        MscInfo& operator=(const MscInfo& info) {
            mscNum = info.mscNum;
            manualLock = info.manualLock;
            automaticLock = info.automaticLock;
            failureCount = info.failureCount;
            return (*this);
        }

        string  mscNum;
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

