#ifndef SCAG_PERS_VARRECSIZE_STORE
#define SCAG_PERS_VARRECSIZE_STORE

#include <inttypes.h>
#include <string>

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/PageFile.hpp"
#include "core/buffers/TmpBuf.hpp"

namespace scag{ namespace pers{

using namespace std;

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;


typedef TmpBuf<char, 2048> _SerialBuffer;
class SerialBuffer : public _SerialBuffer
{
public:
	SerialBuffer() : _SerialBuffer(2048) {};
	SerialBuffer(int size) : _SerialBuffer(size) {};
	string toString()
	{
		string str;
		int i = 0, j = GetPos();
		char buf[10];

		uint8_t b;
		SetPos(0);
		while(i++ < j)
		{
			Read((char*)&b, 1);
			sprintf(buf, "%02x(%c)", (int)b, (b > 32 && b < 128 ? b : '*'));
			str += buf;
		}
		SetPos(j);
		return str;
	}
};

class Serializable
{
public:
	virtual void Serialize(SerialBuffer &sb) = 0;
	virtual void Deserialize(SerialBuffer &sb) = 0;
};

/*class DiskHashKey{
public:
    virtual bool operator==(const DiskHashKey& that)const = 0;
    virtual string toString();
    virtual uint32_t Size();
    virtual void Read(File& f);
    virtual void Write(File& f)const;
    virtual uint32_t HashCode(uint32_t attempt)const;
};*/

class VarRecordNotFound{};

template<class DiskKey>
class VarRecSizeStore
{
public:

	VarRecSizeStore() {};
	~VarRecSizeStore() {};

	void init(const string& storeName)
	{
		log = smsc::logger::Logger::getInstance("scag.VarRecSizeStore");

		string idxFile;
		string binFile;

		idxFile = storeName + ".idx";
		binFile = storeName + ".bin";

		if(!File::Exists(idxFile.c_str()))
			dhash.Create(idxFile.c_str(), 1000, false);
		else
			dhash.Open(idxFile.c_str());

		if(!File::Exists(binFile.c_str()))
			pfile.Create(binFile.c_str(), 256, 250);
		else
			pfile.Open(binFile.c_str());
	}

/*	void shutdown() {
		pfile.Close();
		dhash.Close();
	}*/

	void getRecord(const DiskKey& key, Serializable *rec)
	{
		MutexGuard mg(mtx);

		OffsetValue off;
		vector<unsigned char> data;

		if(!dhash.LookUp(key, off))
			throw VarRecordNotFound();

		pfile.Read(off.value, data);
		SerialBuffer sb(data.size());
		sb.Append((char*)&data[0], data.size());
		sb.SetPos(0);
		rec->Deserialize(sb);

		smsc_log_debug(log, "getRecord: %s, offset=%08llx, size=%d", key.toString().c_str(), off.value, data.size());
//		smsc_log_debug(log, "getRecord:%s", sb.toString().c_str());
	}

	void newRecord(const DiskKey& key, Serializable *rec)
	{
		MutexGuard mg(mtx);

		SerialBuffer sb;
		rec->Serialize(sb);
		OffsetValue val(pfile.Append(sb.get(), sb.GetPos()));

		dhash.Insert(key, val);

		smsc_log_debug(log, "newRecord:%s, offset=%08llx, size=%d", key.toString().c_str(), val.value, sb.GetPos());
//		smsc_log_debug(log, "newRecord:%s", sb.toString().c_str());
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

	void updateRecord(const DiskKey& key, Serializable *rec)
	{
		MutexGuard mg(mtx);

		OffsetValue off;

		if(!dhash.LookUp(key, off))
			throw VarRecordNotFound();

		SerialBuffer sb;
		rec->Serialize(sb);
		pfile.Update(off.value, sb.get(), sb.GetPos());

		smsc_log_debug(log, "updateRecord:%s:%08llx", key.toString().c_str(), off.value);
//		smsc_log_debug(log, "updateRecord:%s", sb.toString().c_str());
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
