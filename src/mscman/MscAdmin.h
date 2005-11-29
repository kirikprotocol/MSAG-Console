
#ifndef SMSC_MSCMAN_MSC_ADMIN
#define SMSC_MSCMAN_MSC_ADMIN

#include <string>

#include "core/buffers/Array.hpp"
#include "core/buffers/File.hpp"

#include "MscExceptions.h"

namespace smsc { namespace mscman
{
    using std::string;
    using smsc::core::buffers::Array;
    using smsc::core::buffers::File;


    struct MscInfo
    {
        MscInfo(const char* msc="", bool mLock=false, bool aLock=false, int fc=0)
            : manualLock(mLock),
                automaticLock(aLock), failureCount(fc)
            {
              offset=-1;
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
            offset=info.offset;
            return (*this);
        }

        void setMscNum(const char* msc)
        {
          strncpy(mscNum,msc,sizeof(mscNum)-1);
          mscNum[sizeof(mscNum)-1]=0;
        }

        void Read(File& f)
        {
          offset=f.Pos();
          f.Read(mscNum,sizeof(mscNum));
          manualLock=f.ReadByte()==1;
          automaticLock=f.ReadByte()==1;
          failureCount=f.ReadNetInt32();
        }

        void Write(File& f)const
        {
          f.Write(mscNum,sizeof(mscNum));
          f.WriteByte(manualLock?1:0);
          f.WriteByte(automaticLock?1:0);
          f.WriteNetInt32(failureCount);
        }

        static int Size()
        {
          return sizeof(MscInfo::mscNum)+1+1+4;
        }

        char    mscNum[22];
        bool    manualLock;
        bool    automaticLock;
        uint32_t     failureCount;
        File::offset_type offset;
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
