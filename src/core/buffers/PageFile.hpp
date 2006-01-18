#ifndef __SMSC_CORE_BUFFERS_PAGEFILE_HPP__
#define __SMSC_CORE_BUFFERS_PAGEFILE_HPP__

#include "util/Exception.hpp"
#include "core/buffers/File.hpp"
#include <stdio.h>
#include "util/int.h"
#include <string>
#include <vector>


namespace smsc{
namespace core{
namespace buffers{

using namespace smsc::core::buffers;


class PageFile{
public:
  PageFile():sig("PaGeFiLe")
  {
    pageHeaderSize=1+4+8;
  }
  void Create(const std::string& argFilename,int argPageSize,int preAllocate=1024)
  {
    filename=argFilename;
    pageSize=argPageSize;
    file.RWCreate(filename.c_str());
    FileFlushGuard fg(file);
    file.Write(sig,8);
    file.WriteNetInt32(pageSize);
    char zero[256-4-8-16]={0,};
    file.Write(zero,sizeof(zero));
    firstFreePage=file.Pos()+16;
    file.WriteNetInt64(firstFreePage);//offset of first free page
    lastFreePage=firstFreePage+pageSize*(preAllocate-1);
    file.WriteNetInt64(lastFreePage);
    headerSize=(int)file.Pos();
    for(int i=0;i<preAllocate;i++)
    {
      file.WriteByte(i==preAllocate-1?pageFreeLast:pageFree);//state
      file.WriteNetInt32(0);//data size
      file.WriteNetInt64(i==preAllocate-1?0:file.Pos()-1-4+pageSize);//net page offset
      file.ZeroFill(pageSize-pageHeaderSize);
    }
  }
  void Open(const std::string& argFilename)
  {
    char fileSig[9]={0,};
    filename=argFilename;
    file.RWOpen(filename.c_str());
    FileFlushGuard fg(file);
    file.Read(fileSig,8);
    if(strcmp(fileSig,sig)!=0)
    {
      throw smsc::util::Exception("Invalid page file signature:%s",filename.c_str());
    }
    pageSize=file.ReadNetInt32();
    file.SeekCur(256-4-8-16);
    firstFreePage=file.ReadNetInt64();
    lastFreePage=file.ReadNetInt64();
    headerSize=(int)file.Pos();
  }
  File::offset_type Read(File::offset_type pageIdx,std::vector<unsigned char>& data,File::offset_type* realIdx=0)
  {
    data.resize(0);
    if(pageIdx==0)pageIdx=headerSize;
    if((pageIdx-headerSize)%pageSize!=0)
    {
      throw smsc::util::Exception("PageFile::Read : Invalid page index:%lld",pageIdx);
    }
    File::offset_type fileSize=file.Size();
    file.Seek(pageIdx);
    for(;;pageIdx+=pageSize)
    {
      if(pageIdx==fileSize)return 0;
      uint8_t state=file.ReadByte();
      if(state!=pageUsedSingle && state!=pageUsedFirst)
      {
        file.Seek(pageIdx+pageSize);
        continue;
      }
      if(realIdx)*realIdx=pageIdx;
      int dataSize=file.ReadNetInt32();
      if(state==pageUsedSingle)
      {
        //printf("Read::Single pageIdx=%08llx\n",pageIdx);
        data.resize(dataSize);
        file.SeekCur(8);
        file.Read(&data[0],dataSize);
      }else
      if(state==pageUsedFirst)
      {
        for(;;)
        {
          File::offset_type next=file.ReadNetInt64();
          //printf("Read::Multi state=%d, pageIdx=%08llx,next=%08llx\n",state,pageIdx,next);
          int sz=data.size();
          data.resize(sz+dataSize);
          file.Read(&data[sz],dataSize);
          if(state==pageUsedFinal)
          {
            break;
          }
          file.Seek(next);
          state=file.ReadByte();
          dataSize=file.ReadNetInt32();
        }
      }
      break;
    }
    return pageIdx+pageSize;
  }
  File::offset_type Append(void* data,unsigned size)
  {
    FileFlushGuard fg(file);
    File::offset_type firstPage=0;
    File::offset_type nextIdx=0;
    while(size>0)
    {
      File::offset_type pageIdx=nextIdx?nextIdx:NewPage();
      nextIdx=size>pageSize-pageHeaderSize?NewPage():0;
      file.Seek(pageIdx);
      int state=pageUsedContinued;
      if(firstPage==0)
      {
        if(!nextIdx)
        {
          state=pageUsedSingle;
        }else
        {
          state=pageUsedFirst;
        }
      }else
      {
        if(!nextIdx)
        {
          state=pageUsedFinal;
        }
      }
      file.WriteByte(state);
      unsigned pieceSize=size>pageSize-pageHeaderSize?pageSize-pageHeaderSize:size;
      file.WriteNetInt32(pieceSize);
      file.WriteNetInt64(nextIdx);
      file.Write(data,pieceSize);
      data=(char*)data+pieceSize;
      size-=pieceSize;
      if(!firstPage)
      {
        firstPage=pageIdx;
      }
    }
    if(nextIdx!=0)
    {
      throw smsc::util::Exception("shit... nextIdx==%lld, size=0");
    }
    return firstPage;
  }

  void Delete(File::offset_type pageIdx)
  {
    FileFlushGuard fg(file);
    bool firstPage=true;
    int status;
    do{
      if((pageIdx-headerSize)%pageSize!=0)
      {
        throw smsc::util::Exception("PageFile::Delete : Invalid page index:%08llx",pageIdx);
      }
      file.Seek(pageIdx);
      status=file.ReadByte();
      //printf("del:%08llx(%d)\n",pageIdx,status);
      file.SeekCur(4);
      File::offset_type nxt=file.ReadNetInt64();
      if((firstPage && status!=pageUsedSingle && status!=pageUsedFirst) ||
         (!firstPage && status!=pageUsedContinued && status!=pageUsedFinal))
      {
        throw smsc::util::Exception("PageFile::Delete : Invalid page index for Delete operation:%08llx",pageIdx);
      }
      FreePage(pageIdx);
      pageIdx=nxt;
      firstPage=false;
    }while(status!=pageUsedFinal && status!=pageUsedSingle);
  }

  void Update(File::offset_type pageIdx,void* data,unsigned size)
  {
    FileFlushGuard fg(file);
    if((pageIdx-headerSize)%pageSize!=0)
    {
      throw smsc::util::Exception("PageFile::Update : Invalid page index:%08llx",pageIdx);
    }
    bool firstPage=true;
    File::offset_type nextIdx=0;
    bool wasNewPage=false;
    while(size>0)
    {
      if(pageIdx==0)
      {
        abort();
      }
      file.Seek(pageIdx);
      int status=file.ReadByte();
      unsigned dataSize=file.ReadNetInt32();
      nextIdx=file.ReadNetInt64();
      if(wasNewPage || status==pageUsedFinal)nextIdx=0;

      if((status==pageUsedSingle || firstPage) && size<=pageSize-pageHeaderSize)
      {
        //printf("Update:: single, idx=%08llx\n",pageIdx);
        file.Seek(pageIdx);
        file.WriteByte(pageUsedSingle);
        file.WriteNetInt32(size);
        file.WriteNetInt64(0);
        file.Write(data,size);
        if(status==pageUsedSingle)return;
        break;
      }

      if(size>pageSize-pageHeaderSize)
      {
        status=firstPage?pageUsedFirst:pageUsedContinued;
      }else if(firstPage && size<=pageSize-pageHeaderSize)
      {
        status=pageUsedSingle;
      }else if(!firstPage && size<=pageSize-pageHeaderSize)
      {
        status=pageUsedFinal;
      }

      if((status==pageUsedContinued || status==pageUsedFirst) && nextIdx==0)
      {
        nextIdx=NewPage();
        wasNewPage=true;
        //printf("nextIdx=NewPage()==%08llx\n",nextIdx);
      }

      file.Seek(pageIdx);
      file.WriteByte(status);
      unsigned pieceSize=size>pageSize-pageHeaderSize?pageSize-pageHeaderSize:size;
      file.WriteNetInt32(pieceSize);
      file.WriteNetInt64(status==pageUsedFinal || status==pageUsedSingle?0:nextIdx);
      file.Write(data,pieceSize);
      //printf("Update:: multi, status=%d, idx=%08llx, size=%u, piece=%d\n",status,pageIdx,size,pieceSize);
      data=(char*)data+pieceSize;
      size-=pieceSize;
      pageIdx=nextIdx;
      firstPage=false;
    }
    //printf("Update: nextIdx after:%08llx\n",nextIdx);
    if(nextIdx)
    {
      int status;
      do
      {
        file.Seek(nextIdx);
        status=file.ReadByte();
        if(status!=pageUsedContinued && status!=pageUsedFinal)
        {
          throw smsc::util::Exception("Invalid page status:%d for FreePage operation at %08llx",status,pageIdx);
        }
        file.SeekCur(4);
        File::offset_type nxt=file.ReadNetInt64();
        FreePage(nextIdx);
        nextIdx=nxt;
        //printf("Update: nextIdx del %08llx\n",nextIdx);
        if(status==pageUsedFinal && nextIdx!=0)
        {
          throw smsc::util::Exception("wtf, status==pageUsedFinal && nextIdx!=0 !");
        }
      }while(status!=pageUsedFinal);
    }
  }

protected:
  File::offset_type NewPage()
  {
    File::offset_type rv;
    if(firstFreePage)
    {
      file.Seek(firstFreePage+1+4);
      rv=firstFreePage;
      firstFreePage=file.ReadNetInt64();
      file.Seek(headerSize-16);
      file.WriteNetInt64(firstFreePage);
      if(!firstFreePage)lastFreePage=0;
    }else
    {
      file.SeekEnd(0);
      rv=file.Pos();
      file.WriteByte(pageFreeLast);
      file.WriteNetInt32(0);
      file.WriteNetInt64(0);
      file.ZeroFill(pageSize-1-4-8);
    }
    //file.Seek(rv+1+4);
    //file.WriteNetInt64(0);
    return rv;
  }

  void FreePage(File::offset_type pageIdx)
  {
    if(lastFreePage)
    {
      file.Seek(lastFreePage);
      file.WriteByte(pageFree);
      file.WriteNetInt32(0);
      file.WriteNetInt64(pageIdx);
    }
    lastFreePage=pageIdx;
    file.Seek(pageIdx);
    file.WriteByte(pageFreeLast);
    file.WriteNetInt32(0);
    file.WriteNetInt64(0);
    if(!firstFreePage)
    {
      file.Seek(headerSize-8-8);
      firstFreePage=pageIdx;
      file.WriteNetInt64(firstFreePage);
    }else
    {
      file.Seek(headerSize-8);
    }
    file.WriteNetInt64(lastFreePage);
  }

  enum{
    pageFree,pageFreeLast,pageUsedSingle,pageUsedFirst,pageUsedContinued,pageUsedFinal
  };
  int pageSize;
  std::string filename;
  File file;
  File::offset_type firstFreePage;
  File::offset_type lastFreePage;
  int headerSize;
  int pageHeaderSize;
  const char* sig;
  PageFile(const PageFile&);
};

}//namespace buffers
}//namespace core
}//namespace smsc

#endif
