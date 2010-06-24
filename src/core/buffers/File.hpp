#ifndef __FILE_HPP__
#define __FILE_HPP__

#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#else
#  if _FILE_OFFSET_BITS!=64
#  error _FILE_OFFSET_BITS must be 64!!!
#  endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <string.h>
#include <errno.h>
#include "util/sleep.h"
#include "util/int.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <vector>
#include <string>

#include "core/buffers/TmpBuf.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <io.h>
#define lseek _lseek
#define O_LARGEFILE 0
#else
# include <netinet/in.h>
# include <inttypes.h>
# include <unistd.h>
#endif



namespace smsc{
namespace core{
namespace buffers{

//using namespace std;

#define _TO_STR2_(x) #x
#define _TO_STR_(x) _TO_STR2_(x)

#define RTERROR(txt) throw std::runtime_error(txt " at " __FILE__ ":" _TO_STR_(__LINE__))

class FileException:public std::exception{
public:
  enum{
    errOpenFailed,errReadFailed,errEndOfFile,
    errWriteFailed,errSeekFailed,errFileNotOpened,
    errRenameFailed,errUnlinkFailed,errFlushFailed,
    errMkDirFailed,errRmDirFailed,
    errOpenDirFailed,errReadDirFailed
  };
  FileException(int err):errCode(err)
  {
    error=errno;
  }
  FileException(int err,const char* fn):errCode(err),fileName(fn)
  {
    error=errno;
  }
  FileException(const FileException& e)
  {
    errCode=e.errCode;
    error=e.error;
  }
  FileException& operator=(const FileException& e)throw()
  {
    errCode=e.errCode;
    error=e.error;
    return *this;
  }
  virtual ~FileException()throw(){}
  const char* what()const throw()
  {
    if(fileName.length())
    {
      errbuf="(";
      errbuf+=fileName;
      errbuf+=")";
    }
    switch(errCode)
    {
      case errOpenFailed:
        errbuf="Failed to open file"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errReadFailed:
        errbuf="Failed to read from file"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errEndOfFile:
        errbuf="End of file reached"+errbuf;
        return errbuf.c_str();
      case errWriteFailed:
        errbuf="Failed to write to file"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errSeekFailed:
        errbuf="Failed to change file pointer"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errFileNotOpened:
        errbuf="Attempt to make an operation on file that is not opened"+errbuf;
        return errbuf.c_str();
      case errRenameFailed:
        errbuf="Failed to rename file"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errUnlinkFailed:
        errbuf="Failed to unlink file"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errMkDirFailed:
        errbuf="Failed to create directory"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errRmDirFailed:
        errbuf="Failed to remove directory"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errOpenDirFailed:
        errbuf="Failed to open directory"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      case errReadDirFailed:
        errbuf="Failed to read directory"+errbuf;
        errbuf+=":";
        errbuf+=strerror(error);
        return errbuf.c_str();
      default: return "Unknown error";
    }
  }
  int getErrorCode(){return errCode;}
  int getErrNo(){return error;}
protected:
  FileException();
  int errCode;
  int error;
  std::string fileName;
  mutable std::string errbuf;
};

class File;

class GlobalFileEventHandler{
public:
  virtual void onCreateFileObject(File*)=0;
  virtual void onDestroyFileObject(File*)=0;
  virtual void onRename(const char* oldName,const char* newName)=0;
  virtual void onUnlink(const char* fileName)=0;
  virtual void onMkDir(const char* dirName,int mode)=0;
  virtual void onRmDir(const char* dirName)=0;

  virtual ~GlobalFileEventHandler(){}
  static void setGlobalFileEventHandler(GlobalFileEventHandler* hnd)
  {
    StaticHolder<0>::handler=hnd;
  }
  static GlobalFileEventHandler* getGlobalFileEventHandler()
  {
    return StaticHolder<0>::handler;
  }

protected:
  template <int N=0>
  struct StaticHolder
  {
    static GlobalFileEventHandler* handler;
  };
};

template <int N>
GlobalFileEventHandler* GlobalFileEventHandler::StaticHolder<N>::handler=0;



class FileEventHandler{
public:
  virtual ~FileEventHandler(){}
  enum{
    openRead,openWrite,openRW,openCreate,openAppend
  };
  virtual void onOpen(int mode,const char* fileName)=0;
  virtual void onRead(const void* data,size_t sz)=0;
  virtual void onWrite(const void* data,size_t sz)=0;
  virtual void onSeek(int whence,int64_t offset)=0;
  virtual void onRename(const char* newName)=0;
};

class File{
public:
  typedef int64_t offset_type;
  File():fd(-1)
  {
    flags=FLG_BUFFERED;
    buffer=initBuffer;
    bufferSize=sizeof(initBuffer);
    bufferPosition=0;
    maxFlushSpeed=0;
    fileSize=0;
    eventHandler=0;
    if(GlobalFileEventHandler::getGlobalFileEventHandler())
    {
      GlobalFileEventHandler::getGlobalFileEventHandler()->onCreateFileObject(this);
    }
  }
  ~File()
  {
    Close();
    if(GlobalFileEventHandler::getGlobalFileEventHandler())
    {
      GlobalFileEventHandler::getGlobalFileEventHandler()->onDestroyFileObject(this);
    }
  }

  void Swap(File& swp)
  {
    if(flags&FLG_RDBUF)flags&=~FLG_RDBUF;
    if(flags&FLG_WRBUF)Flush();
    if(swp.flags&FLG_RDBUF)swp.flags&=~FLG_RDBUF;
    if(swp.flags&FLG_WRBUF)swp.Flush();

    if(buffer!=initBuffer || swp.buffer!=swp.initBuffer)
    {
      std::swap(buffer,swp.buffer);
    }

    std::swap(fd,swp.fd);
    std::swap(bufferSize,swp.bufferSize);
    std::swap(fileSize,swp.fileSize);
    std::swap(bufferPosition,swp.bufferPosition);
    std::swap(filename,swp.filename);
    std::swap(flags,swp.flags);
    std::swap(eventHandler,swp.eventHandler);
  }

  void SetEventHandler(FileEventHandler* handler)
  {
    eventHandler=handler;
  }

  FileEventHandler* GetEventHandler()
  {
    return eventHandler;
  }


  void SwapBuffers(File& swp)
  {
    if(isInMemory())
    {
      std::swap(buffer,swp.buffer);
      std::swap(bufferSize,swp.bufferSize);
      std::swap(fileSize,swp.fileSize);
      std::swap(bufferPosition,swp.bufferPosition);
    }else
    {
      throw std::runtime_error("Attempt to swap buffers for file not in memory");
    }
  }

  void OpenInMemory(int sz)
  {
    if(isInMemory())return;
    if(isBuffered())
    {
      Flush();
      if(buffer!=initBuffer)delete [] buffer;
      flags&=~FLG_BUFFERED;
      bufferSize=0;
      bufferPosition=0;
    }
    if(fd!=-1)
    {
      sz=(int)Size();
      fileSize=sz;
      bufferPosition=(int)Pos();
    }else
    {
      bufferPosition=0;
      fileSize=0;
    }

    if(sz<0 || sz>MAX_BUFFER_SIZE)throw std::runtime_error("File buffer size exceeded limit");

    if(sz==0)sz=1024;

    flags|=FLG_INMEMORY;
    buffer=new char[sz];
    bufferSize=sz;

    if(fd!=-1)
    {
      lseek(fd,0,SEEK_SET);
      size_t rdsz=read(fd,buffer,(size_t)fileSize);
      if(rdsz!=fileSize)throw FileException(FileException::errReadFailed,filename.c_str());
      if(eventHandler)
      {
        eventHandler->onRead(buffer,(size_t)fileSize);
      }
    }
  }

  void MemoryFlush()
  {
    if(!isInMemory() || fd==-1)throw FileException(FileException::errFileNotOpened,filename.c_str());
    std::string tmp=filename+".tmp";
    int g=open(tmp.c_str(),O_WRONLY | O_CREAT | O_TRUNC| O_LARGEFILE,0644);
    if(g==-1)throw FileException(FileException::errOpenFailed,filename.c_str());
    offset_type written=0;

    int blockSize=8192;
    if(maxFlushSpeed>0)
    {
      blockSize=maxFlushSpeed;
    }
    uint64_t writeTime;
    if(eventHandler)
    {
      eventHandler->onSeek(SEEK_SET,0);
    }
    do{
      if(maxFlushSpeed>0)writeTime=smsc::util::getmillis();
      size_t piece=(size_t)(fileSize-written<blockSize?fileSize-written:blockSize);

      if(write(g,buffer+written,(size_t)piece)!=(ssize_t)piece)
      {
        close(g);
        remove(tmp.c_str());
        throw FileException(FileException::errWriteFailed,filename.c_str());
      }
      if(eventHandler)
      {
        eventHandler->onWrite(buffer+written,(size_t)piece);
      }
      written+=piece;
      if(maxFlushSpeed>0)
      {
        writeTime=smsc::util::getmillis()-writeTime;
        if(writeTime<1000)smsc::util::millisleep((unsigned)(1000-writeTime));
      }
    }while(written<fileSize);
    close(fd);
    fd=-1;
    std::string old=filename+".old";
    if(rename(filename.c_str(),old.c_str())!=0)
    {
      throw FileException(FileException::errRenameFailed,filename.c_str());
    }
    if(rename(tmp.c_str(),filename.c_str())!=0)
    {
      throw FileException(FileException::errRenameFailed,filename.c_str());
    }
    remove(old.c_str());
    fd=g;
    lseek(fd,0,SEEK_SET);
  }

  void DiscardCache()
  {
    if(!isInMemory() || fd==-1)throw FileException(FileException::errFileNotOpened,filename.c_str());
    flags&=~FLG_INMEMORY;
    size_t sz=(int)Size();
    if(bufferSize<sz)
    {
      if(buffer!=initBuffer)delete [] buffer;
      buffer=new char[sz];
      bufferSize=sz;
    }
    flags|=FLG_INMEMORY;
    lseek(fd,0,SEEK_SET);
    if(read(fd,buffer,sz)!=ssize_t(sz))throw FileException(FileException::errReadFailed,filename.c_str());
    fileSize=sz;
    bufferPosition=0;
  }

  const std::string& getFileName() const
  {
    return filename;
  }


  void ROpen(const char* fn)
  {
    Close();
    filename=fn;
    fd=open(fn,O_RDONLY|O_LARGEFILE,0644);
    if(fd==-1)throw FileException(FileException::errOpenFailed,fn);
    if(eventHandler)eventHandler->onOpen(FileEventHandler::openRead,fn);
  }
  void WOpen(const char* fn)
  {
    Close();
    filename=fn;
    fd=open(fn,O_WRONLY|O_LARGEFILE|O_CREAT,0644);
    if(fd==-1)throw FileException(FileException::errOpenFailed,fn);
    if(eventHandler)eventHandler->onOpen(FileEventHandler::openWrite,fn);
  }
  void RWOpen(const char* fn)
  {
    Close();
    filename=fn;
    fd=open(fn,O_RDWR|O_LARGEFILE,0644);
    if(fd==-1)throw FileException(FileException::errOpenFailed,fn);
    if(eventHandler)eventHandler->onOpen(FileEventHandler::openRW,fn);
  }
  void RWCreate(const char* fn)
  {
    Close();
    filename=fn;
    fd=open(fn,O_CREAT|O_RDWR|O_TRUNC|O_LARGEFILE,0644);
    if(fd==-1)throw FileException(FileException::errOpenFailed,fn);
    if(eventHandler)eventHandler->onOpen(FileEventHandler::openCreate,fn);
  }
  void Append(const char* fn)
  {
    Close();
    filename=fn;
    fd=open(fn,O_CREAT|O_WRONLY|O_APPEND|O_LARGEFILE,0644);
    if(fd==-1)throw FileException(FileException::errOpenFailed,fn);
    if(eventHandler)eventHandler->onOpen(FileEventHandler::openAppend,fn);
    SeekEnd(0);
  }

  void SetUnbuffered()
  {
    if(fd!=-1)Flush();
    flags&=~FLG_BUFFERED;
    ResetBuffer();
  }

  void SetAutoBuffer(int sz=0)
  {
    if(sz!=0)
    {
      SetBuffer(sz);
    }
    flags|=FLG_AUTOBUFFERED;
  }

  void SetBuffer(int sz)
  {
    if(sz<int(bufferSize))return;
    if(fd!=-1)Flush();
    ResetBuffer();
    buffer=new char[sz];
    bufferSize=sz;
    flags|=FLG_BUFFERED;
  }

  void Close()
  {
    if(fd!=-1)
    {
      try{
        Flush();
      }catch(std::exception& e)
      {
        flags&=~FLG_WRBUF;
      }
      close(fd);
      fd=-1;
      if(isBuffered())
      {
        ResetBuffer();
        fileSize=0;
      }
    }
    if(isInMemory())
    {
      delete [] buffer;
      flags&=~FLG_INMEMORY;
    }
  }

  bool isOpened()
  {
    return fd!=-1;
  }

  size_t Read(void* buf,size_t sz)
  {
    if(isInMemory())
    {
      if(bufferPosition+sz>fileSize)
      {
        throw FileException(FileException::errEndOfFile,filename.c_str());
      }
      memcpy(buf,buffer+bufferPosition,sz);
      bufferPosition+=sz;
      return sz;
    }
    Check();
    if(flags&FLG_WRBUF)Flush();
    if((flags&FLG_BUFFERED) && (flags&FLG_RDBUF)!=FLG_RDBUF && sz<(size_t)bufferSize)
    {
      ssize_t rd=read(fd,buffer,bufferSize);
      if(rd==-1)
      {
        throw FileException(FileException::errReadFailed,filename.c_str());
      }
      bufferUsed=rd;
      if(eventHandler)eventHandler->onRead(buffer,bufferSize);
      bufferPosition=0;
      flags|=FLG_RDBUF;
    }

    if(flags&FLG_RDBUF)
    {
      if(bufferPosition+sz<bufferUsed)
      {
        memcpy(buf,buffer+bufferPosition,sz);
        bufferPosition+=sz;
        return sz;
      }
      size_t avail=bufferUsed-bufferPosition;
      memcpy(buf,buffer+bufferPosition,avail);
      sz-=avail;
      char* tmp=(char*)buf;
      tmp+=avail;
      buf=tmp;
      if(sz<bufferSize)
      {
        ssize_t rd=read(fd,buffer,bufferSize);
        if(rd==-1)
        {
          throw FileException(FileException::errReadFailed,filename.c_str());
        }
        bufferUsed=rd;
        if(bufferUsed<sz)
          throw FileException(FileException::errEndOfFile,filename.c_str());
        if(eventHandler)eventHandler->onRead(buffer,bufferUsed);
        memcpy(buf,buffer,sz);
        bufferPosition=sz;
        return sz+avail;
      }
      bufferPosition=0;
      bufferUsed=0;

    }

    size_t rv=read(fd,buf,sz);
    if(rv!=sz)
    {
      if(errno)
        throw FileException(FileException::errReadFailed,filename.c_str());
      else
      {
        throw FileException(FileException::errEndOfFile,filename.c_str());
      }
    }
    if(eventHandler)eventHandler->onRead(buf,sz);
    return rv;
  }

  template <class T>
  offset_type XRead(T& t)
  {
    return Read(&t,sizeof(T));
  }
  void Write(const void* buf,size_t sz)
  {
    if(isInMemory())
    {
      if(bufferPosition+sz>bufferSize)
      {
        ResizeBuffer(bufferPosition+sz);
      }
      memcpy(buffer+bufferPosition,buf,sz);
      bufferPosition+=sz;
      if(bufferPosition>fileSize)fileSize=bufferPosition;
      return;
    }
    Check();
    if(flags&FLG_RDBUF)
    {
      lseek(fd,-(bufferUsed-bufferPosition),SEEK_CUR);
      if(eventHandler)eventHandler->onSeek(SEEK_CUR,-(bufferUsed-bufferPosition));
      flags&=~FLG_RDBUF;
    }

    if(flags&FLG_BUFFERED)
    {
      if(!(flags&FLG_WRBUF))
      {
        bufferPosition=0;
      }
      flags|=FLG_WRBUF;

      if(flags&FLG_AUTOBUFFERED)
      {
        if(bufferPosition+sz>bufferSize)
        {
          ResizeBuffer(bufferPosition+sz);
        }
      }
      if(bufferPosition+sz>bufferSize)
      {
        size_t towr=bufferSize-bufferPosition;
        memcpy(buffer+bufferPosition,buf,towr);
        const char* tmp=(const char*)buf;
        tmp+=towr;
        buf=tmp;
        sz-=towr;
        bufferPosition=0;
        if(write(fd,buffer,bufferSize)!=ssize_t(bufferSize))
        {
          throw FileException(FileException::errWriteFailed,filename.c_str());
        }
        if(eventHandler)eventHandler->onWrite(buffer,bufferSize);
        if(sz>bufferSize)
        {
          if(write(fd,buf,sz)!=ssize_t(sz))
          {
            throw FileException(FileException::errWriteFailed,filename.c_str());
          }
          if(eventHandler)eventHandler->onWrite(buf,sz);
        }else
        {
          memcpy(buffer,buf,sz);
          bufferPosition=sz;
        }
      }else
      {
        memcpy(buffer+bufferPosition,buf,sz);
        bufferPosition+=sz;
      }
    }else
    {
      if(write(fd,buf,sz)!=ssize_t(sz))throw FileException(FileException::errWriteFailed,filename.c_str());
      if(eventHandler)eventHandler->onWrite(buf,sz);
    }

  }
  template <class T>
  void XWrite(const T& t)
  {
    Write(&t,sizeof(T));
  }

  void ZeroFill(size_t sz)
  {
    if(isInMemory())
    {
      if(bufferPosition+sz>bufferSize)
      {
        ResizeBuffer(bufferPosition+sz);
      }
      memset(buffer+bufferPosition,0,sz);
      bufferPosition+=sz;
      if(bufferPosition>fileSize)fileSize=bufferPosition;
      return;
    }
    char buf[8192]={0,};
    size_t blksz;
    while(sz>0)
    {
      blksz=sz>sizeof(buf)?sizeof(buf):sz;
      Write(buf,blksz);
      sz-=blksz;
    }
  }

  void Seek(offset_type off,int whence=SEEK_SET)
  {
    Check();
    if(isInMemory())
    {
      switch(whence)
      {
        case SEEK_SET:
        {
            bufferPosition=(int)off;
        }break;
        case SEEK_END:
        {
            bufferPosition=(int)fileSize+(int)off;
        }break;
        case SEEK_CUR:
        {
            bufferPosition+=(int)off;
        }break;
        default:
        {
          throw std::runtime_error("invalid whence parameter");
        }
      }
      if(bufferPosition<0)bufferPosition=0;
      if(bufferPosition>bufferSize)ResizeBuffer(bufferPosition);
    }else
    {
      Flush();
      if(flags&FLG_RDBUF)
      {
        if(whence==SEEK_CUR)
        {
          if(bufferPosition+off>0 && bufferPosition+off<bufferUsed)
          {
            bufferPosition+=off;
            return;
          }
          off-=bufferUsed-bufferPosition;
        }
        flags&=~FLG_RDBUF;

      }
      if(lseek(fd,off,whence)==-1)throw FileException(FileException::errSeekFailed,filename.c_str());
      if(eventHandler)eventHandler->onSeek(whence,off);
    }
  }

  void SeekEnd(offset_type off)
  {
    Seek(off,SEEK_END);
  }

  void SeekCur(offset_type off)
  {
    Seek(off,SEEK_CUR);
  }

  offset_type Size()
  {
    if(isInMemory())return fileSize;
    Check();
    Flush();
#ifdef __GNUC__
    struct stat st;
#else
    struct ::stat st;
#endif
    fstat(fd,&st);
    return st.st_size;
  }

  offset_type Pos()
  {
    if(isInMemory())return bufferPosition;
    Check();
    offset_type realPos=lseek(fd,0,SEEK_CUR);
    if(flags&FLG_RDBUF)
    {
      return realPos-bufferUsed+bufferPosition;
    }
    if(flags&FLG_WRBUF)
    {
      return realPos+bufferPosition;
    }
    return realPos;
  }

  void Truncate( offset_type pos )
  {
      if (isInMemory()) { throw std::runtime_error("Truncate is not impl for inmemory");}
      Check();
      if (flags & (FLG_RDBUF|FLG_WRBUF)) {
          throw std::runtime_error("Truncate is not impl for buffered file");
      }
      if (-1 == ftruncate(fd,pos)) {
          throw FileException(FileException::errWriteFailed,filename.c_str());
      }
  }

  uint64_t ReadInt64()
  {
    uint64_t t;
    XRead(t);
    return t;
  }

  uint64_t ReadNetInt64()
  {
    uint32_t h=ReadNetInt32();
    uint32_t l=ReadNetInt32();
    return (((uint64_t)h)<<32)|l;
  }

  uint32_t ReadInt32()
  {
    uint32_t t;
    XRead(t);
    return t;
  }

  uint32_t ReadNetInt32()
  {
    return ntohl(ReadInt32());
  }

  uint16_t ReadInt16()
  {
    uint16_t t;
    XRead(t);
    return t;
  }
  uint16_t ReadNetInt16()
  {
    return ntohs(ReadInt16());
  }

  uint8_t ReadByte()
  {
    uint8_t b;
    XRead(b);
    return b;
  }

  void WriteInt32(uint32_t t)
  {
    XWrite(t);
  }
  void WriteNetInt32(uint32_t t)
  {
    WriteInt32(htonl(t));
  }

  void WriteInt16(uint16_t t)
  {
    XWrite(t);
  }
  void WriteNetInt16(uint16_t t)
  {
    WriteInt16(htons(t));
  }

  void WriteInt64(uint64_t t)
  {
    XWrite(t);
  }
  void WriteNetInt64(uint64_t t)
  {
    uint32_t h=htonl((uint32_t)((t>>32)&0xFFFFFFFFUL));
    uint32_t l=htonl((uint32_t)(t&0xFFFFFFFFUL));
    XWrite(h);
    XWrite(l);
  }

  void WriteByte(uint8_t t)
  {
    XWrite(t);
  }

  void Flush()
  {
    Check();
    if(flags&FLG_WRBUF)
    {
      if(bufferPosition>0)
      {
        if(maxFlushSpeed==0)
        {
          if(write(fd,buffer,bufferPosition)!=ssize_t(bufferPosition))
            throw FileException(FileException::errWriteFailed,filename.c_str());
          if(eventHandler)eventHandler->onWrite(buffer,bufferPosition);
          bufferPosition=0;
        }else
        {
          uint64_t writeTime;
          size_t written=0;
          while(written<bufferPosition)
          {
            size_t pieceSize=int(bufferPosition-written)<maxFlushSpeed?bufferPosition-written:maxFlushSpeed;
            writeTime=smsc::util::getmillis();
            if(write(fd,buffer+written,pieceSize)!=ssize_t(pieceSize))
              throw FileException(FileException::errWriteFailed,filename.c_str());
            if(eventHandler)eventHandler->onWrite(buffer+written,pieceSize);
            writeTime=smsc::util::getmillis()-writeTime;
            if(writeTime<1000)
            {
              smsc::util::millisleep((unsigned)(1000-writeTime));
            }
            written+=pieceSize;
          }
          bufferPosition=0;
        }
      }
      flags&=~FLG_WRBUF;
    }
  }

  int FSync()
  {
    Check();
    return fsync(fd);
  }

  void EnableDirectIO()
  {
#ifdef __SunOS
    directio(fd,DIRECTIO_ON);
#endif
  }

  void DisableDirectIO()
  {
#ifdef __SunOS
    directio(fd,DIRECTIO_OFF);
#endif
  }


  void setMaxFlushSpeed(int mxflsp)
  {
    maxFlushSpeed=mxflsp;
  }

  bool ReadLine(std::string& str)
  {
    Check();
    if(isInMemory())
    {
      if(bufferPosition==fileSize)return false;
      char* eoln=(char*)memchr(buffer+bufferPosition,'\n',(int)fileSize-bufferPosition);
      if(eoln==NULL)
      {
        str.assign(buffer+bufferPosition,(int)fileSize-bufferPosition);
      }else
      {
        eoln--;
        if(*eoln==0x0d)eoln--;
        str.assign(buffer+bufferPosition,eoln-buffer);
      }
      return true;
    }else
    {
      str="";
      char c;
      offset_type pos=Pos();
      offset_type sz=Size();
      while(pos<sz)
      {
        pos+=Read(&c,1);
        if(c==0x0a)return true;
        str+=c;
      }
      return false;
    }
  }

  template <size_t SZ>
  void WriteFixedString(const char (&str)[SZ])
  {
    char buf[SZ]={0,};
    memcpy(buf,str,std::min((size_t)SZ,strlen(str)));
    Write(buf,SZ);
  }

  template <size_t SZ>
  void WriteFixedString(const std::string& str)
  {
    char buf[SZ]={0,};
    memcpy(buf,str.c_str(),std::min((size_t)SZ,str.length()));
    Write(buf,SZ);
  }

  template <unsigned int SZ>
  void ReadFixedString(char (&str)[SZ])
  {
    Read(str,SZ);
    str[SZ-1]=0;
  }

  template <unsigned int SZ>
  void ReadFixedString(std::string& str)
  {
    char buf[SZ+1];
    buf[SZ-1]=0;
    Read(buf,SZ);
    str=buf;
  }


  void Rename(const char* newname)
  {
    if(rename(filename.c_str(),newname)!=0)throw FileException(FileException::errRenameFailed,filename.c_str());
    filename=newname;
    if(eventHandler)
    {
      eventHandler->onRename(newname);
    }
  }

  static bool Exists(const char* file)
  {
#ifdef _WIN32
    struct _stat st;
    return _stat(file,&st)==0;
#else
#ifdef __GNUC__
    struct stat st;
#else
    struct ::stat st;
#endif
    return ::stat(file,&st)==0;
#endif
  }

    static bool IsDir( const char* file ) {
#ifdef _WIN32
# error "WIN32 is not impl yet"
#endif

#ifdef __GNUC__
        struct stat st;
#else
        struct ::stat st;
#endif
        return (::stat(file,&st)==0) && S_ISDIR(st.st_mode);
    }

  static void Rename(const char* oldname,const char* newname)
  {
    if(rename(oldname,newname)!=0)throw FileException(FileException::errRenameFailed,oldname);
    if(GlobalFileEventHandler::getGlobalFileEventHandler())
    {
      GlobalFileEventHandler::getGlobalFileEventHandler()->onRename(oldname,newname);
    }
  }

  void RenameExt(const char* newext)
  {
    std::string newFileName=filename;
    std::string::size_type pos=newFileName.rfind('.');
    if(pos!=std::string::npos)
    {
      newFileName.erase(pos);
    }
    newFileName+='.';
    newFileName+=newext;
    Rename(newFileName.c_str());
  }

  static void Unlink(const char* fn)
  {
    if(unlink(fn)!=0)throw FileException(FileException::errUnlinkFailed,fn);
    if(GlobalFileEventHandler::getGlobalFileEventHandler())
    {
      GlobalFileEventHandler::getGlobalFileEventHandler()->onUnlink(fn);
    }
  }

  static void MkDir(const char* dirName,int mode=0755)
  {
    if(mkdir(dirName,mode)!=0)throw FileException(FileException::errMkDirFailed,dirName);
    if(GlobalFileEventHandler::getGlobalFileEventHandler())
    {
      GlobalFileEventHandler::getGlobalFileEventHandler()->onMkDir(dirName,mode);
    }
  }

  static void RmDir(const char* dirName)
  {
    if(rmdir(dirName)!=0)throw FileException(FileException::errRmDirFailed,dirName);
    if(GlobalFileEventHandler::getGlobalFileEventHandler())
    {
      GlobalFileEventHandler::getGlobalFileEventHandler()->onRmDir(dirName);
    }
  }

  enum ReadDirFilter{
    rdfAll,
    rdfFilesOnly,
    rdfDirsOnly,
    rdfFullPath=0x1000,
    rdfNoDots=0x2000
  };

  friend File::ReadDirFilter operator|(File::ReadDirFilter a,File::ReadDirFilter b)
  {
    return (File::ReadDirFilter)((int)a|(int)b);
  }


  static void ReadDir(const char* dirName,std::vector<std::string>& dirContents,ReadDirFilter flt=rdfAll|rdfNoDots)
  {
    DIR* dir=opendir(dirName);
    if(!dir)
    {
      throw FileException(FileException::errOpenDirFailed,dirName);
    }
    struct CloseDirGuard{
      DIR* dir;
      CloseDirGuard(DIR* argDir):dir(argDir){}
      ~CloseDirGuard()
      {
        closedir(dir);
      }
    }guard(dir);
    TmpBuf<char,512> buf(sizeof(dirent)+pathconf(dirName,_PC_NAME_MAX)+1);
    dirent* de=(dirent*)buf.get();
    dirent* ptr;
    if(readdir_r(dir,de,&ptr)!=0)
    {
      throw FileException(FileException::errReadDirFailed,dirName);
    }
    bool fullPath=(flt&rdfFullPath)!=0;
    bool noDots=(flt&rdfNoDots)!=0;
    flt=(ReadDirFilter)(flt&0xfff);
    std::string file;
    while(ptr)
    {
      if(noDots && (strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0))
      {
        if(readdir_r(dir,de,&ptr)!=0)
        {
          throw FileException(FileException::errReadDirFailed,dirName);
        }
        continue;
      }
      if(flt!=rdfAll)
      {
        struct ::stat st;
        file=dirName;
        if(file.length() && file[file.length()-1]!='/')file+="/";
        file+=ptr->d_name;
        if(::stat(file.c_str(),&st)!=0)
        {
          throw FileException(FileException::errReadDirFailed,file.c_str());
        }
        if(flt==rdfFilesOnly)
        {
          if((st.st_mode&S_IFREG)!=0)
          {
            dirContents.push_back(fullPath?file:ptr->d_name);
          }
        }else if(flt==rdfDirsOnly)
        {
          if((st.st_mode&S_IFDIR)!=0)
          {
            dirContents.push_back(fullPath?file:ptr->d_name);
          }
        }
      }else
      {
        if(fullPath)
        {
          file=dirName;
          if(file.length() && file[file.length()-1]!='/')file+="/";
          file+=ptr->d_name;
          dirContents.push_back(file);
        }else
        {
          dirContents.push_back(ptr->d_name);
        }
      }
      if(readdir_r(dir,de,&ptr)!=0)
      {
        throw FileException(FileException::errReadDirFailed,dirName);
      }
    }
  }

  static offset_type Size(const char* argFileName)
  {
#ifdef __GNUC__
    struct stat st;
#else
    struct ::stat st;
#endif
    ::stat(argFileName,&st);
    return st.st_size;
  }

protected:
  enum{INIT_BUFFER_SIZE=8192};
  enum{MAX_BUFFER_SIZE=2000*1024*1024};
  enum{
    FLG_BUFFERED=1,
    FLG_RDBUF=2,
    FLG_WRBUF=4,
    FLG_INMEMORY=8,
    FLG_AUTOBUFFERED=16
  };
  char  initBuffer[INIT_BUFFER_SIZE];
  char *buffer;
  size_t bufferSize,bufferUsed;
  size_t bufferPosition;
  offset_type fileSize;
  FileEventHandler* eventHandler;
  std::string filename;
  int   fd;
  int   flags;
  int maxFlushSpeed;

  bool isBuffered()
  {
    return flags&FLG_BUFFERED;
  }

  bool isInMemory()
  {
    return flags&FLG_INMEMORY;
  }

  void Check()
  {
    if(isInMemory())return;
    if(fd==-1)
    {
      throw FileException(FileException::errFileNotOpened,filename.c_str());
    }
  }

  void ResetBuffer()
  {
    if(buffer!=initBuffer)delete [] buffer;
    buffer=initBuffer;
    bufferSize=sizeof(initBuffer);
    bufferPosition=0;
    bufferUsed=0;
  }

  void ResizeBuffer(size_t newsz)
  {
    newsz+=newsz/4;
    if(newsz>MAX_BUFFER_SIZE || newsz<=0)
    {
      char buf[128];
      sprintf(buf,"File buffer size exceeded limit:%ld/%d",long(newsz),MAX_BUFFER_SIZE);
      throw std::runtime_error(buf);
    }
    char* newbuf=new char[newsz];
    memcpy(newbuf,buffer,bufferSize);
    delete [] buffer;
    buffer=newbuf;
    bufferSize=newsz;
  }
};

class FileFlushGuard{
public:
  FileFlushGuard(File& argFile):file(argFile){}
  ~FileFlushGuard(){file.Flush();}
protected:
  File& file;
};

} //namespace buffers
} //namespace core
} //namespace smsc

#endif
