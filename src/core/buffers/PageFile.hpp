#ifndef __SMSC_CORE_BUFFERS_PAGEFILE_HPP__
#define __SMSC_CORE_BUFFERS_PAGEFILE_HPP__

#include "util/Exception.hpp"
#include "core/buffers/File.hpp"
#include <stdio.h>
#include "util/int.h"
#include <string>
#include <vector>

// #define PAGEFILEDEBUG

#ifdef PAGEFILEDEBUG
#include "logger/Logger.h"
#endif


namespace smsc{
namespace core{
namespace buffers{

using namespace smsc::core::buffers;

/// This exception provides some means to recover corrupted PageFile.
/// It could be triggered in case the page chain in the file is
/// broken for some reason.
/// The instance of the exception gives the initial page in the chain
/// and the page where the problem arises.
/// TODO: create a method PageFile::recover( page ).
class PageFileCorruptedException : public smsc::util::Exception
{
public:
    PageFileCorruptedException( File::offset_type failpage,
                                File::offset_type initpage,
                                const char* const fmt, ... ) :
    Exception(), failpage_(failpage), initpage_(initpage) {
        SMSC_UTIL_EX_FILL( fmt );
    }
    File::offset_type failedPage() const {
        return failpage_;
    }
    File::offset_type initialPage() const {
        return initpage_;
    }
private:
    File::offset_type failpage_;
    File::offset_type initpage_;
};

class PageFile{
public:
  PageFile():sig("PaGeFiLe")
  {
    pageHeaderSize=1+4+8;
#ifdef PAGEFILEDEBUG
      log_ = smsc::logger::Logger::getInstance( "pagefile" );
#endif
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
      if ( preAllocate > 0 ) {
          firstFreePage=file.Pos()+16;
          lastFreePage=firstFreePage+pageSize*(preAllocate-1);
      } else {
          firstFreePage = 0;
          lastFreePage = 0;
      }
#ifdef PAGEFILEDEBUG
      smsc_log_debug( log_, "freepages first=%llx last=%llx",
                      static_cast<long long>(firstFreePage),
                      static_cast<long long>(lastFreePage) );
#endif
    file.WriteNetInt64(firstFreePage);//offset of first free page
    file.WriteNetInt64(lastFreePage);
    headerSize=(int)file.Pos();
    for(int i=0;i<preAllocate;i++)
    {
        uint8_t state = (i==preAllocate-1?pageFreeLast:pageFree);
        File::offset_type nextPage = (i==preAllocate-1?0:file.Pos()+pageSize);
#ifdef PAGEFILEDEBUG
        if ( log_->isDebugEnabled() ) showpage( "prealloc", file.Pos(), state, 0, nextPage );
#endif
      file.WriteByte(state);
      file.WriteNetInt32(0);//data size
      file.WriteNetInt64(nextPage);//net page offset
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
      if (!firstFreePage && lastFreePage ) {
          // file was corrupted in NewPage (first written, last not)
#ifdef PAGEFILEDEBUG
          smsc_log_warn( log_, "file was corrupted in NewPage: last=%llx",
                         static_cast<long long>(lastFreePage) );
#endif
          fprintf( stderr, "pagefile was corrupted in NewPage: last=%llx\n",
                   static_cast<long long>(lastFreePage) );
          lastFreePage = 0;
          file.Seek(headerSize-8);
          file.WriteNetInt64( lastFreePage );
      }
      if ( lastFreePage ) {
          file.Seek( lastFreePage );
          uint8_t status = file.ReadByte();
          if ( status != pageFreeLast ) {
              // file was corrupted in FreePage (last was not written)
#ifdef PAGEFILEDEBUG
              if ( log_->isDebugEnabled() ) showpage( "open", lastFreePage, status, -1, -1 );
              smsc_log_warn( log_, "file was corrupted in FreePage: last=%llx, status=%s",
                             static_cast<long long>(lastFreePage),
                             statusstring(status) );
#endif
              fprintf( stderr, "pagefile was corrupted in FreePage: last=%llx, status=%s\n",
                       static_cast<long long>(lastFreePage),
                       statusstring(status) );
              File::offset_type startPage = lastFreePage;
              File::offset_type nextPage = 0;
              for (;;) {
                  file.Seek( lastFreePage + 1 + 4 );
                  nextPage = file.ReadNetInt64();
                  if ( nextPage == startPage ) {
#ifdef PAGEFILEDEBUG
                      smsc_log_warn( log_, "looping at page=%llx, last free page forced",
                                      static_cast<long long>(lastFreePage) );
#endif
                      fprintf( stderr, "looping at page=%llx, last free page forced\n",
                               static_cast<long long>(lastFreePage) );
                      nextPage = 0; // looping
                  }
                  status = ( nextPage ? pageFree : pageFreeLast );
#ifdef PAGEFILEDEBUG
                  if ( log_->isDebugEnabled() ) showpage( "open fix", lastFreePage, status, 0, nextPage );
#endif
                  file.Seek( lastFreePage );
                  file.WriteByte( status );
                  file.WriteNetInt32(0);
                  file.WriteNetInt64( nextPage );
                  if ( nextPage ) 
                      lastFreePage = nextPage;
                  else
                      break;
              }
              file.Seek(headerSize-8);
              file.WriteNetInt64(lastFreePage);
          }
      }
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
#ifdef PAGEFILEDEBUG
          if ( log_->isDebugEnabled() ) showpage( "rd skip", pageIdx, state, -1, -1 );
#endif
        file.Seek(pageIdx+pageSize);
        continue;
      }
      if(realIdx)*realIdx=pageIdx;
      int dataSize=file.ReadNetInt32();
      if(state==pageUsedSingle)
      {
        //printf("Read::Single pageIdx=%08llx\n",pageIdx);
// #ifdef PAGEFILEDEBUG
//          if ( log_->isDebugEnabled() ) showpage( "rd single", pageIdx, state, dataSize, 0 );
// #endif
        data.resize(dataSize);
        file.SeekCur(8);
        file.Read(&data[0],dataSize);
      }else
      if(state==pageUsedFirst)
      {
          File::offset_type innerPageIdx = pageIdx;
        for(;;)
        {
          File::offset_type next=file.ReadNetInt64();
          //printf("Read::Multi state=%d, pageIdx=%08llx,next=%08llx\n",state,pageIdx,next);
// #ifdef PAGEFILEDEBUG
//             if ( log_->isDebugEnabled() ) showpage( "rd multi", innerPageIdx, state, dataSize, next );
// #endif
          size_t sz=data.size();
          data.resize(sz+dataSize);
          file.Read(&data[sz],dataSize);
          if(state==pageUsedFinal)
          {
            break;
          } else if ( state != pageUsedFirst && state != pageUsedContinued ) {
              // file corrupted in previous Append / Update
#ifdef PAGEFILEDEBUG
              smsc_log_warn( log_, "file corrupted in Append/Update: page=%llx state=%s size=%u next=%llx",
                             static_cast<long long>(innerPageIdx),
                             statusstring(state),
                             dataSize,
                             static_cast<long long>(next) );
#endif
              throw PageFileCorruptedException( innerPageIdx,
                                                pageIdx,
                                                "file corrupted: wrong state of page=%llx state=%s",
                                                static_cast<long long>(innerPageIdx), statusstring(state) );
          }
          file.Seek(next);
          innerPageIdx = next;
          state=file.ReadByte();
          dataSize=file.ReadNetInt32();
        }
      }
      break;
    }
    return pageIdx+pageSize;
  }
  File::offset_type Append(const void* data,unsigned size)
  {
    FileFlushGuard fg(file);
    File::offset_type firstPage=0;
    File::offset_type nextIdx=0;
    while(size>0)
    {
      File::offset_type pageIdx=nextIdx?nextIdx:NewPage();
      nextIdx=size>unsigned(pageSize-pageHeaderSize)?NewPage():0;
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
      unsigned pieceSize=size>unsigned(pageSize-pageHeaderSize)?pageSize-pageHeaderSize:size;
#ifdef PAGEFILEDEBUG
        if ( log_->isDebugEnabled() ) showpage( "append", pageIdx, state, pieceSize, nextIdx );
#endif
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

  /// Delete the entry pages starting from pageIdx.
  /// @return number of pages actually deleted
  unsigned Delete(File::offset_type pageIdx)
  {
    FileFlushGuard fg(file);
    bool firstPage=true;
    int status;
    unsigned delPages = 0;
    do{
      if((pageIdx-headerSize)%pageSize!=0)
      {
        throw smsc::util::Exception( "PageFile::Delete : Invalid page index:%08llx",pageIdx);
      }
      file.Seek(pageIdx);
      status=file.ReadByte();
      //printf("del:%08llx(%d)\n",pageIdx,status);
      file.SeekCur(4);
      File::offset_type nxt=file.ReadNetInt64();
#ifdef PAGEFILEDEBUG
      if ( log_->isDebugEnabled() ) showpage("delete", pageIdx, status, -1, nxt );
#endif
      if((firstPage && status!=pageUsedSingle && status!=pageUsedFirst) ||
         (!firstPage && status!=pageUsedContinued && status!=pageUsedFinal))
      {
          // file was corrupted in previous Append/Update
          throw PageFileCorruptedException( pageIdx, pageIdx,
                                            "PageFile::Delete : Invalid page index for Delete operation:%08llx", pageIdx );
      }
      FreePage(pageIdx);
      ++delPages;
      pageIdx=nxt;
      firstPage=false;
    }while(status!=pageUsedFinal && status!=pageUsedSingle);
    return delPages;
  }

  void Update(File::offset_type pageIdx,const void* data,unsigned size)
  {
    FileFlushGuard fg(file);
      File::offset_type initPageIdx = pageIdx;
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
          // FIXME: for now I leave this abort()
        abort();
          // file was corrupted in previous Append/Update ?
          throw PageFileCorruptedException( nextIdx, initPageIdx, 
                                            "file corrupted: size>0 but nextpage=0 at page=%llx",
                                            static_cast<long long>(nextIdx) );
      }
      file.Seek(pageIdx);
      int status=file.ReadByte();
      unsigned dataSize=file.ReadNetInt32();
      nextIdx=file.ReadNetInt64();
#ifdef PAGEFILEDEBUG
      if ( log_->isDebugEnabled() ) showpage("upd read", pageIdx, status, dataSize, nextIdx );
#endif
      if(wasNewPage || status==pageUsedFinal)nextIdx=0;

      if((status==pageUsedSingle || firstPage) && size<=unsigned(pageSize-pageHeaderSize))
      {
        //printf("Update:: single, idx=%08llx\n",pageIdx);
        file.Seek(pageIdx);
        file.WriteByte(pageUsedSingle);
        file.WriteNetInt32(size);
        file.WriteNetInt64(0);
        file.Write(data,size);
#ifdef PAGEFILEDEBUG
        if ( log_->isDebugEnabled() ) showpage("upd writ", pageIdx, pageUsedSingle, size, 0 );
#endif
        if(status==pageUsedSingle)return;
        break;
      }

      if(size>unsigned(pageSize-pageHeaderSize))
      {
        status=firstPage?pageUsedFirst:pageUsedContinued;
      }else if(firstPage && size<=unsigned(pageSize-pageHeaderSize))
      {
        status=pageUsedSingle;
      }else if(!firstPage && size<=unsigned(pageSize-pageHeaderSize))
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
      unsigned pieceSize=size>unsigned(pageSize-pageHeaderSize)?pageSize-pageHeaderSize:size;
      file.WriteNetInt32(pieceSize);
      {
          File::offset_type nextPage = (status==pageUsedFinal || status==pageUsedSingle?0:nextIdx);
#ifdef PAGEFILEDEBUG
          if ( log_->isDebugEnabled() ) showpage("upd writ", pageIdx, status, pieceSize, nextPage );
#endif
          file.WriteNetInt64(nextPage);
      }
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
            // file was corrupted in previous Append/Update
            throw PageFileCorruptedException( nextIdx, nextIdx,
                                     "Invalid page status:%s for FreePage operation at %08llx",
                                     statusstring(status), nextIdx );
        }
        file.SeekCur(4);
        File::offset_type nxt=file.ReadNetInt64();
#ifdef PAGEFILEDEBUG
        if ( log_->isDebugEnabled() ) showpage("upd tail", nextIdx, status, -1, nxt );
#endif
        FreePage(nextIdx);
        nextIdx=nxt;
        //printf("Update: nextIdx del %08llx\n",nextIdx);
        if(status==pageUsedFinal && nextIdx!=0)
        {
            // file was corrupted in previous Append/Update
            throw PageFileCorruptedException( nextIdx, nextIdx, "wtf, status==pageUsedFinal && nextIdx!=0 !" );
        }
      } while(status!=pageUsedFinal);
    }
  }

    const int getPageSize() const { return pageSize; }

protected:
  File::offset_type NewPage()
  {
    File::offset_type rv;
#ifdef PAGEFILEDEBUG
    smsc_log_debug( log_, "newpage: first=%llx last=%llx", firstFreePage, lastFreePage );
#endif
    if(firstFreePage)
    {
      file.Seek(firstFreePage+1+4);
      rv=firstFreePage;
      firstFreePage=file.ReadNetInt64();
#ifdef PAGEFILEDEBUG
      if ( log_->isDebugEnabled() ) showpage("new first", rv, -1, -1, firstFreePage );
#endif
      file.Seek(headerSize-16);
      file.WriteNetInt64(firstFreePage);
      if(!firstFreePage) {
          lastFreePage=0;
          file.WriteNetInt64(lastFreePage);
      }
    }else
    {
      file.SeekEnd(0);
      rv=file.Pos();
#ifdef PAGEFILEDEBUG
      if ( log_->isDebugEnabled() ) showpage("new toend", rv, pageFreeLast, 0, 0 );
#endif
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
#ifdef PAGEFILEDEBUG
    smsc_log_debug( log_, "freepage: first=%llx last=%llx", firstFreePage, lastFreePage );
#endif
    if(lastFreePage)
    {
#ifdef PAGEFILEDEBUG
      if ( log_->isDebugEnabled() ) showpage("free last", lastFreePage, pageFree, 0, pageIdx );
#endif
      file.Seek(lastFreePage);
      file.WriteByte(pageFree);
      file.WriteNetInt32(0);
      file.WriteNetInt64(pageIdx);
    }
    lastFreePage=pageIdx;
#ifdef PAGEFILEDEBUG
    if ( log_->isDebugEnabled() ) showpage("free new", pageIdx, pageFreeLast, 0, 0 );
#endif
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


#ifdef PAGEFILEDEBUG
    void showpage( const char* where, File::offset_type pageIdx,
                   uint8_t state, unsigned dataSize, File::offset_type nextIdx ) const {
        char ds[30];
        if ( dataSize != unsigned(-1) ) {
            snprintf( ds, sizeof(ds), "%u", dataSize );
        } else {
            strcpy( ds, "???" );
        }
        char nx[60];
        if ( nextIdx != File::offset_type(-1) ) {
            snprintf( nx, sizeof(nx), "%llx", 
                      static_cast<long long>(nextIdx) );
        } else {
            strcpy( nx, "???" );
        }
        smsc_log_debug( log_, "%s pg=%llx stat=%s sz=%s next=%s",
        where, static_cast<long long>(pageIdx), statusstring(state), ds, nx );
    }
#endif

    const char* statusstring( uint8_t state ) const
    {
        switch ( state ) {
        case (pageFree) :
            return "free";
        case (pageFreeLast) :
            return "freelast";
        case (pageUsedSingle) :
            return "usedsingle";
        case (pageUsedFirst) :
            return "usedfirst";
        case (pageUsedContinued) :
            return "usedcontd";
        case (pageUsedFinal) :
            return "usedfinal";
        };
        return "???";
    }

  int pageSize;
  std::string filename;
  File file;
  File::offset_type firstFreePage;
  File::offset_type lastFreePage;
  int headerSize;
  int pageHeaderSize;
  const char* sig;
#ifdef PAGEFILEDEBUG
    smsc::logger::Logger* log_;
#endif
  PageFile(const PageFile&);
};

}//namespace buffers
}//namespace core
}//namespace smsc

#ifdef PAGEFILEDEBUG
#undef PAGEFILEDEBUG
#endif

#endif
