#ifndef __SMSC_CORE_BUFFERS_FIXEDRECORDFILE_HPP__
#define __SMSC_CORE_BUFFERS_FIXEDRECORDFILE_HPP__

#include "core/buffers/File.hpp"
#include "util/int.h"
#include "core/buffers/TmpBuf.hpp"
#include <vector>
#include "util/Exception.hpp"

namespace smsc{
namespace core{
namespace buffers{

/*

class RecordType MUST have following members:
  void Read(File& file);
  void Write(File& file)const;
  static uint32_t Size();

*/

template <class RecordType>
class FixedRecordFile{
public:
  FixedRecordFile(const char* argSig,uint32_t argVer):sig(argSig),
    sigLen((int)strlen(argSig)),ver(argVer),filePos(0),fileSize(0)
  {
  }

  void Open(const char* fileName)
  {
    if(!File::Exists(fileName))
    {
      storeFile.RWCreate(fileName);
      storeFile.Write(sig,sigLen);
      storeFile.WriteNetInt32(ver);
      storeFile.Flush();
      filePos=storeFile.Pos();
      fileSize=filePos;
      return;
    }
    storeFile.RWOpen(fileName);
    TmpBuf<char,16> fileSig(sigLen+1);
    storeFile.Read(fileSig.get(),sigLen);
    if(memcmp(fileSig.get(),sig,sigLen)!=0)
    {
      throw smsc::util::Exception("Invalid signature in file %s",fileName);
    }
    uint32_t fileVer;
    fileVer=storeFile.ReadNetInt32();
    if(fileVer!=ver)
    {
      throw smsc::util::Exception("Invalid version of file %s",fileName);
    }
    filePos=storeFile.Pos();
    fileSize=storeFile.Size();
  }

  void Close()
  {
    storeFile.Close();
  }

  File::offset_type Read(RecordType& rec)
  {
    if(filePos==fileSize || (File::offset_type)(filePos+RecordType::Size())>fileSize)return 0;
    bool used;
    File::offset_type rv=0;
    do{
      used=storeFile.ReadByte()!=0;
      if(used)
      {
        rv=filePos;
        rec.Read(storeFile);
      }else
      {
        holes.push_back(filePos);
        storeFile.SeekCur(RecordType::Size());
      }
      filePos+=1+RecordType::Size();
    }while(!used && (File::offset_type)(filePos+RecordType::Size())<=fileSize);
    if(!used && filePos!=fileSize)
    {
      holes.push_back(filePos);
    }
    return rv;
  }

  void Write(File::offset_type off,const RecordType& rec)
  {
    storeFile.Seek(off+1);
    rec.Write(storeFile);
    storeFile.Flush();
  }

  File::offset_type Append(const RecordType& rec)
  {
    File::offset_type rv=fileSize;
    if(!holes.empty())
    {
      rv=holes.back();
      holes.pop_back();
    }else
    {
      fileSize+=1+RecordType::Size();
    }
    storeFile.Seek(rv);
    storeFile.WriteByte(1);
    rec.Write(storeFile);
    storeFile.Flush();
    return rv;
  }

  void Delete(File::offset_type off)
  {
    if(off==0)throw smsc::util::Exception("Invalid offset for FixedRecordFile::Delete operation");
    storeFile.Seek(off);
    storeFile.WriteByte(0);
    storeFile.Flush();
    holes.push_back(off);
  }

protected:
  const char* sig;
  const int sigLen;
  const uint32_t ver;
  std::vector<File::offset_type> holes;
  File storeFile;
  File::offset_type filePos,fileSize;
};

}//buffers
}//core
}//smsc

#endif
