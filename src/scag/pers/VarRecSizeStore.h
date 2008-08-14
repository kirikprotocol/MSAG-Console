/* $Id$ */

#ifndef SCAG_PERS_VARRECSIZE_STORE
#define SCAG_PERS_VARRECSIZE_STORE

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/PageFile.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include <vector>

using std::vector;

namespace scag{ namespace pers{

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;

using scag::util::storage::Serializable;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

class VarRecordNotFound{};

template<class DiskKey>
class VarRecSizeStore
{
public:

    VarRecSizeStore() {};
    ~VarRecSizeStore() {};

    void init(const std::string& storeName, uint32_t initRecCnt)
    {
        log = smsc::logger::Logger::getInstance("VarStore");

        std::string idxFile;
        std::string binFile;

        idxFile = storeName + ".idx";
        binFile = storeName + ".bin";

        if(!File::Exists(idxFile.c_str()))
            dhash.Create(idxFile.c_str(), initRecCnt, false);
        else
            dhash.Open(idxFile.c_str());

        if(!File::Exists(binFile.c_str()))
            pfile.Create(binFile.c_str(), 256, initRecCnt);
        else
            pfile.Open(binFile.c_str());
    }

    bool getRecord(const DiskKey& key, Serializable* rec)
    {
        MutexGuard mg(mtx);

        OffsetValue off;
        vector<unsigned char> data;

        if(!dhash.LookUp(key, off))
            return false;

        pfile.Read(off.value, data);
        SerialBuffer sb(data.size());
        sb.Append((char*)&data[0], data.size());
        sb.SetPos(0);
        
        try{
            rec->Deserialize(sb, true);
        }
        catch(SerialBufferOutOfBounds &f) {
            smsc_log_error(log, "Seems storage is corrupted. Profile %s.", key.toString().c_str());
            return false;
        }

        smsc_log_debug(log, "getRecord: %s, offset=%08llx, size=%d", key.toString().c_str(), off.value, data.size());
//      smsc_log_debug(log, "getRecord:%s", sb.toString().c_str());
        return true;
    }

    void newRecord(const DiskKey& key, Serializable* rec)
    {
        MutexGuard mg(mtx);

        SerialBuffer sb;
        rec->Serialize(sb, true);
        OffsetValue val(pfile.Append((void*)sb.c_ptr(), sb.length()));

        dhash.Insert(key, val);

        smsc_log_debug(log, "newRecord:%s, offset=%08llx, size=%d", key.toString().c_str(), val.value, sb.GetPos());
//      smsc_log_debug(log, "newRecord:%s", sb.toString().c_str());
    }

    void deleteRecord(const DiskKey& key)
    {
        MutexGuard mg(mtx);

        OffsetValue off;
        if(!dhash.LookUp(key, off))
        {
            smsc_log_info(log, "Attempt to delete record that doesn't exists:%s", key.toString().c_str());
            return;
        }

        pfile.Delete(off.value);
        dhash.Delete(key);

        smsc_log_debug(log, "delRecord:%s:%08llx", key.toString().c_str(), off.value);
    }

    bool updateRecord(const DiskKey& key, Serializable* rec)
    {
        MutexGuard mg(mtx);

        OffsetValue off;

        if(!dhash.LookUp(key, off))
            return false;

        SerialBuffer sb;
        rec->Serialize(sb, true);
        pfile.Update(off.value, (void*)sb.c_ptr(), sb.length());

        smsc_log_debug(log, "updateRecord:%s:%08llx", key.toString().c_str(), off.value);
//      smsc_log_debug(log, "updateRecord:%s", sb.toString().c_str());
        return true;
    }

protected:
    smsc::logger::Logger* log;

    struct OffsetValue{
        File::offset_type value;

        OffsetValue():value(0){}
        OffsetValue(File::offset_type argValue):value(argValue){}
        OffsetValue(const OffsetValue& src):value(src.value){}

        static uint32_t Size(){return sizeof(File::offset_type);}
        void Read(File& f)
        {
            value=f.ReadNetInt64();
        }
        void Write(File& f)const
        {
            f.WriteNetInt64(value);
        }
    };

    PageFile pfile;
    DiskHash<DiskKey, OffsetValue> dhash;
    Mutex mtx;
};

}}

#endif // SCAG_PERS_VARRECSIZE_STORE
