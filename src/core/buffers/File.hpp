#ifndef __FILE_HPP__
#define __FILE_HPP__

#define __USE_FILE_OFFSET64
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
#include "util/int.h"
#include "util/debug.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <winsock2.h>
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

#define RTERROR(txt) runtime_error(txt " at " __FILE__ ":" _TO_STR_(__LINE__))

class FileException:public std::exception{
public:
  enum{
    errOpenFailed,errReadFailed,errEndOfFile,
    errWriteFailed,errSeekFailed,errFileNotOpened,
    errRenameFailed,errUnlinkFailed
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
  ~FileException()throw(){}
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

class File{
public:
#ifdef _WIN32
  typedef int64_t offset_type;
#else
  typedef long long offset_type;
#endif
  File():f(0),inMemoryFile(false),autoFlush(false){}
  ~File()
  {
    Close();
  }

  void Swap(File& swp)
  {
    std::swap(f,swp.f);
    std::swap(inMemoryFile,swp.inMemoryFile);
    std::swap(buffer,swp.buffer);
    std::swap(bufferSize,swp.bufferSize);
    std::swap(fileSize,swp.fileSize);
    std::swap(bufferPosition,swp.bufferPosition);
    std::swap(filename,swp.filename);
    std::swap(autoFlush,swp.autoFlush);
  }

  void SwapBuffers(File& swp)
  {
    if(inMemoryFile)
    {
      std::swap(buffer,swp.buffer);
      std::swap(bufferSize,swp.bufferSize);
      std::swap(fileSize,swp.fileSize);
      std::swap(bufferPosition,swp.bufferPosition);
    }
  }

  void OpenInMemory(int sz)
  {
    if(inMemoryFile)return;
    if(f)
    {
      sz=(int)Size();
      fileSize=sz;
      bufferPosition=(int)Pos();
    }else
    {
      bufferPosition=0;
      fileSize=0;
    }

    if(sz<0 || sz>500*1024*1024)abort();

    if(sz==0)sz=1024;

    inMemoryFile=true;
    buffer=new char[sz];
    bufferSize=sz;

    if(f)
    {
      fseek(f,0,SEEK_SET);
      fread(buffer,sz,1,f);
    }
  }

  void MemoryFlush()
  {
    if(!inMemoryFile || !f)throw FileException(FileException::errFileNotOpened,filename.c_str());
    std::string tmp=filename+".tmp";
    __trace2__("File: memflush %s->%s",filename.c_str(),tmp.c_str());
    FILE *g=fopen(tmp.c_str(),"wb+");
    if(!g)throw FileException(FileException::errOpenFailed,filename.c_str());
    if(fwrite(buffer,fileSize,1,g)!=1)
    {
      fclose(g);
      remove(tmp.c_str());
      throw FileException(FileException::errWriteFailed,filename.c_str());
    }
    fclose(f);
    f=0;
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
    f=g;
    fseek(f,0,SEEK_SET);
  }

  void DiscardCache()
  {
    if(!inMemoryFile || !f)throw FileException(FileException::errFileNotOpened,filename.c_str());
    inMemoryFile=false;
    int sz=(int)Size();
    __trace2__("File: discard %s, oldsz=%d, newsz=%d",filename.c_str(),fileSize,sz);
    if(bufferSize<sz)
    {
      delete [] buffer;
      buffer=new char[sz];
      bufferSize=sz;
    }
    inMemoryFile=true;
    fseek(f,0,SEEK_SET);
    if(fread(buffer,sz,1,f)!=1)throw FileException(FileException::errReadFailed,filename.c_str());
    fileSize=sz;
    bufferPosition=0;
  }

  const std::string& getFileName()
  {
    return filename;
  }


  void ROpen(const char* fn)
  {
    Close();
    filename=fn;
    f=fopen(fn,"rb");
    if(f==NULL)throw FileException(FileException::errOpenFailed,fn);
  }
  void WOpen(const char* fn)
  {
    Close();
    filename=fn;
    f=fopen(fn,"wb");
    if(f==NULL)throw FileException(FileException::errOpenFailed,fn);
  }
  void RWOpen(const char* fn)
  {
    Close();
    filename=fn;
    f=fopen(fn,"rb+");
    if(f==NULL)throw FileException(FileException::errOpenFailed,fn);
  }
  void RWCreate(const char* fn)
  {
    Close();
    filename=fn;
    f=fopen(fn,"wb+");
    if(f==NULL)throw FileException(FileException::errOpenFailed,fn);
  }
  void Append(const char* fn)
  {
    Close();
    filename=fn;
    f=fopen(fn,"ab");
    if(f==NULL)throw FileException(FileException::errOpenFailed,fn);
  }
  void SetUnbuffered()
  {
    autoFlush=true;
  }
  void Close()
  {
    if(f)
    {
      fclose(f);
      f=0;
    }
    if(inMemoryFile)
    {
      delete [] buffer;
      inMemoryFile=false;
    }
  }


  int Read(void* buf,size_t sz)
  {
    if(inMemoryFile)
    {
      if(bufferPosition+sz>fileSize)
      {
        //abort();
        throw FileException(FileException::errEndOfFile,filename.c_str());
      }
      memcpy(buf,buffer+bufferPosition,sz);
      bufferPosition+=sz;
      return sz;
    }
    Check();
    size_t rv=fread(buf,1,sz,f);
    if(rv!=sz)
    {
      if(ferror(f))
        throw FileException(FileException::errReadFailed,filename.c_str());
      else
      {
        //abort();
        throw FileException(FileException::errEndOfFile,filename.c_str());
      }
    }
    return rv;
  }
  template <class T>
  int XRead(T& t)
  {
    return Read(&t,sizeof(T));
  }
  void Write(const void* buf,size_t sz)
  {
    if(inMemoryFile)
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
    if(fwrite(buf,sz,1,f)!=1)throw FileException(FileException::errWriteFailed,filename.c_str());
    if(autoFlush)fflush(f);
  }
  template <class T>
  void XWrite(const T& t)
  {
    Write(&t,sizeof(T));
  }

  void ZeroFill(int sz)
  {
    if(inMemoryFile)
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
    int blksz;
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
    if(whence==SEEK_SET)
    {
      if(inMemoryFile)
      {
        bufferPosition=(int)off;
      }else
      {
        fpos_t p(off);
        if(fsetpos(f,&p)!=0)throw FileException(FileException::errSeekFailed,filename.c_str());
      }
    }else if(whence==SEEK_END)
    {
      if(inMemoryFile)
      {
        bufferPosition=fileSize+(int)off;
      }else
      {
        fseek(f,0,SEEK_END);
        fpos_t p;
        fgetpos(f,&p);
        p+=off;
        if(fsetpos(f,&p)!=0)throw FileException(FileException::errSeekFailed,filename.c_str());
      }
    }else if(whence==SEEK_CUR)
    {
      if(inMemoryFile)
      {
        bufferPosition+=(int)off;
      }else
      {
        fpos_t p;
        fgetpos(f,&p);
        p+=off;
        if(fsetpos(f,&p)!=0)throw FileException(FileException::errSeekFailed,filename.c_str());
      }
    }else
    {
      throw std::runtime_error("invalid whence parameter");
    }
    if(inMemoryFile)
    {
      if(bufferPosition<0)bufferPosition=0;
      if(bufferPosition>bufferSize)ResizeBuffer(bufferPosition);
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
    if(inMemoryFile)return fileSize;
    Check();
    fpos_t p;
    fgetpos(f,&p);
    SeekEnd(0);
    fpos_t rv;
    fgetpos(f,&rv);
    fsetpos(f,&p);
    return rv;
  }

  offset_type Pos()
  {
    if(inMemoryFile)return bufferPosition;
    Check();
    fpos_t p;
    fgetpos(f,&p);
    return p;
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

  void Flush()
  {
    Check();
    if(!inMemoryFile)fflush(f);
  }

  bool ReadLine(std::string& str)
  {
    Check();
    if(inMemoryFile)
    {
      if(bufferPosition==fileSize)return false;
      char* eoln=(char*)memchr(buffer+bufferPosition,'\n',fileSize-bufferPosition);
      if(eoln==NULL)
      {
        str.assign(buffer+bufferPosition,fileSize-bufferPosition);
      }else
      {
        eoln--;
        if(*eoln==0x0d)eoln--;
        str.assign(buffer+bufferPosition,eoln-buffer);
      }
      return true;
    }else
    {
      offset_type off=Pos();
      offset_type fsz=Size();
      char buf[1024];
      char *bufptr=buf;
      int bufsize=1024;
      for(;;)
      {
        if(fgets(bufptr,bufsize,f)==NULL)return false;
        int len=strlen(bufptr);
        if(bufptr[len-1]=='\n' || feof(f))
        {
          if(bufptr[len-1]=='\n')
          {
            len--;
            if(len>0 && bufptr[len-1]==0x0d)len--;
          }
          str.assign(bufptr,len);
          if(bufptr!=buf)delete [] bufptr;
          return true;
        }
        bufsize*=2;
        fsetpos(f,&off);
        if(bufptr!=buf)delete [] bufptr;
        bufptr=new char[bufsize];
      }
    }
  }

  void Rename(const char* newname)
  {
    if(rename(filename.c_str(),newname)!=0)throw FileException(FileException::errRenameFailed,filename.c_str());
    filename=newname;
  }

  static bool Exists(const char* file)
  {
#ifdef _WIN32
    struct _stat st;
    return _stat(file,&st)==0;
#else
    struct ::stat st;
    return ::stat(file,&st)==0;
#endif
  }

  static void Rename(const char* oldname,const char* newname)
  {
    if(rename(oldname,newname)!=0)throw FileException(FileException::errRenameFailed,oldname);
  }

  static void Unlink(const char* fn)
  {
    if(unlink(fn)!=0)throw FileException(FileException::errUnlinkFailed,fn);
  }

protected:
  FILE *f;
  bool  inMemoryFile;
  char *buffer;
  int   bufferSize,fileSize;
  int   bufferPosition;
  std::string filename;
  bool  autoFlush;
  void Check()
  {
    if(inMemoryFile)return;
    if(!f)throw FileException(FileException::errFileNotOpened,filename.c_str());
  }

  void ResizeBuffer(int newsz)
  {
    newsz+=newsz/4;
    if(newsz>500*1024*1024 || newsz<=0)abort();
    char* newbuf=new char[newsz];
    memcpy(newbuf,buffer,bufferSize);
    delete [] buffer;
    buffer=newbuf;
    bufferSize=newsz;
  }
};

} //namespace buffers
} //namespace core
} //namespace smsc

#endif
