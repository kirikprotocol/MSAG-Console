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
#include <exception>
#include <stdexcept>
#include <string>
#include <errno.h>
#include "util/int.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>
#endif


namespace smsc{
namespace core{
namespace buffers{

using namespace std;

#define _TO_STR2_(x) #x
#define _TO_STR_(x) _TO_STR2_(x)

#define RTERROR(txt) runtime_error(txt " at " __FILE__ ":" _TO_STR_(__LINE__))

class FileException:public exception{
public:
  enum{errOpenFailed,errReadFailed,errEndOfFile,errWriteFailed,errSeekFailed,errFileNotOpened};
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
      default: return "Unknown error";
    }
  }
  int getErrorCode(){return errCode;}
  int getErrNo(){return error;}
protected:
  FileException();
  int errCode;
  int error;
  string fileName;
  mutable string errbuf;
};

class File{
public:
#ifdef _WIN32
  typedef int64_t offset_type;
#else
  typedef long long offset_type;
#endif
  File():f(0){}
  ~File()
  {
    Close();
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
    setbuf(f,0);
  }
  void Close()
  {
    if(f)fclose(f);
  }
  int Read(void* buf,size_t sz)
  {
    Check();
    size_t rv=fread(buf,1,sz,f);
    if(rv!=sz)
    {
      if(ferror(f))
        throw FileException(FileException::errReadFailed,filename.c_str());
      else
        throw FileException(FileException::errEndOfFile,filename.c_str());
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
    Check();
    if(fwrite(buf,sz,1,f)!=1)throw FileException(FileException::errWriteFailed,filename.c_str());
  }
  template <class T>
  void XWrite(const T& t)
  {
    Write(&t,sizeof(T));
  }

  void ZeroFill(int sz)
  {
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
      fpos_t p(off);
      if(fsetpos(f,&p)!=0)throw FileException(FileException::errSeekFailed,filename.c_str());
    }else if(whence==SEEK_END)
    {
      fseek(f,0,SEEK_END);
      fpos_t p;
      fgetpos(f,&p);
      p+=off;
      if(fsetpos(f,&p)!=0)throw FileException(FileException::errSeekFailed,filename.c_str());
    }else if(whence==SEEK_CUR)
    {
      fpos_t p;
      fgetpos(f,&p);
      p+=off;
      if(fsetpos(f,&p)!=0)throw FileException(FileException::errSeekFailed,filename.c_str());
    }else
    {
      throw runtime_error("invalid whence parameter");
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
    uint32_t h=htonl((t>>32)&0xFFFFFFFFUL);
    uint32_t l=htonl(t&0xFFFFFFFFUL);
    XWrite(h);
    XWrite(l);
  }

  static bool Exists(const char* file)
  {
#ifdef _WIN32
    struct _stat st;
    return _stat(file,&st)==0;
#else
    struct stat st;
    return stat(file,&st)==0;
#endif
  }

protected:
  FILE *f;
  string filename;
  void Check()
  {
    if(!f)throw FileException(FileException::errFileNotOpened,filename.c_str());
  }
};

} //namespace buffers
} //namespace core
} //namespace smsc

#endif
