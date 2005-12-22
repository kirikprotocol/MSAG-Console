#ifndef __CHUNKFILE_HPP__
#define __CHUNKFILE_HPP__

#include "File.hpp"
#include "util/crc32.h"
#include <memory>
#include <string>

namespace smsc{
namespace core{
namespace buffers{


static const uint32_t _cf_magic=0x4348464c; //CHFL
static const uint32_t _cf_ch_magic=0x43484e4b; //CHNK
static const uint32_t _cf_version=0x00010000;

/*
  expected record interface:
  default constructor
  static uint32_t Size();
  void Read(File&)throw(exception);
  void Write(File&)const throw(exception);
  static void WriteBadValue(File&)throw(exception);

  //bool isBadValue();

  expected performance data struct:
  RootChunks //number of chunk headers in root catalogue
  ChunkRecordsCount  //number of records in chunk
*/

typedef File::offset_type ChunkId;

template <class R>
class ChunkFile{
protected:

  struct FileHeader{
    uint32_t          magic;
    uint32_t          version;
    uint32_t          flags;
    File::offset_type lastRootOffset;
    uint32_t          hdrcrc32;

    void Read(File& f)
    {
      magic=f.ReadNetInt32();
      version=f.ReadNetInt32();
      flags=f.ReadNetInt32();
      lastRootOffset=f.ReadNetInt64();
      hdrcrc32=f.ReadNetInt32();
    }

    static uint32_t fldcrc(uint32_t crc,uint32_t fld)
    {
      uint32_t no=htonl(fld);
      return crc32(crc,&no,4);
    }

    static uint32_t fldcrc(uint32_t crc,uint64_t fld)
    {
      uint32_t low=(fld&0xFFFFFFFFLL);
      uint32_t high=(fld>>32)&0xFFFFFFFFLL;
      uint32_t no=htonl(high);
      crc=crc32(crc,&no,4);
      no=htonl(low);
      return crc32(crc,&no,4);
    }


    void Write(File& f)
    {
      hdrcrc32=fldcrc(0,magic);
      hdrcrc32=fldcrc(hdrcrc32,version);
      hdrcrc32=fldcrc(hdrcrc32,flags);
      hdrcrc32=fldcrc(hdrcrc32,(uint64_t)lastRootOffset);
      f.WriteNetInt32(magic);
      f.WriteNetInt32(version);
      f.WriteNetInt32(flags);
      f.WriteNetInt64(lastRootOffset);
      f.WriteNetInt32(hdrcrc32);
    }
    static uint32_t Size()
    {
      return 4*4+sizeof(File::offset_type);
    }
  };

  struct RootRecord{
    uint32_t size;
    uint32_t count;
    File::offset_type nextPage;
    void Read(File& f)
    {
      size=f.ReadNetInt32();
      count=f.ReadNetInt32();
      nextPage=f.ReadNetInt64();
    }
    void Write(File& f)
    {
      f.WriteNetInt32(size);
      f.WriteNetInt32(count);
      f.WriteNetInt64(nextPage);
    }
    static uint32_t Size()
    {
      return 4+4+sizeof(File::offset_type);
    }
  };

  struct ChunkStartItem{
    File::offset_type firstPage;
    File::offset_type lastPage;
    void Read(File& f)
    {
      firstPage=f.ReadNetInt64();
      lastPage=f.ReadNetInt64();
    }
    void Write(File& f)
    {
      f.WriteNetInt64(firstPage);
      f.WriteNetInt64(lastPage);
    }
    static uint32_t Size()
    {
      return sizeof(File::offset_type)+sizeof(File::offset_type);
    }
  };

  struct ChunkPageHeader{
    uint32_t magic;
    uint32_t size;
    uint32_t count;
    uint32_t padding;
    File::offset_type nextPage;

    bool operator==(const ChunkPageHeader& item)
    {
      return nextPage==item.nextPage;
    }

    void Read(File& f)
    {
      magic=f.ReadNetInt32();
      size=f.ReadNetInt32();
      count=f.ReadNetInt32();
      padding=f.ReadNetInt32();
      nextPage=f.ReadNetInt64();
      if(magic!=_cf_ch_magic)RTERROR("Invalid chunk magic");
    }
    void Write(File& f)
    {
      f.WriteNetInt32(magic);
      f.WriteNetInt32(size);
      f.WriteNetInt32(count);
      f.WriteNetInt32(padding);
      f.WriteNetInt64(nextPage);
    }
    static uint32_t Size()
    {
      return 4*4+sizeof(File::offset_type);
    }
  };

  File f;
  bool opened;
  bool readOnly;
  bool isCached;

  File::offset_type fileSize;
  File::offset_type lastRootOffset;
  RootRecord lastRoot;
  int rootSize;

  ChunkFile(const ChunkFile&);
  void operator=(const ChunkFile&);

public:
  ChunkFile()
  {
    fileSize=0;
    opened=false;
    readOnly=false;
  }
  ~ChunkFile()
  {
    Close();
  }
  void Open(const char* file,bool readonly=true,bool cached=false)
  {
    if(readonly)
      f.ROpen(file);
    else
    {
      std::string old=file;
      old+=".old";
      if(!File::Exists(file) && File::Exists(old.c_str()))
      {
        rename(old.c_str(),file);
      }
      f.RWOpen(file);
    }
    f.SetUnbuffered();
    isCached=cached;
    if(cached)f.OpenInMemory(0);
    FileHeader h;
    h.Read(f);
    if(h.magic!=_cf_magic)RTERROR("invalid file magic");
    if(h.version!=_cf_version)RTERROR("incompatible chunk file version");
    uint32_t crc=crc32(0,&h,h.Size()-sizeof(uint32_t));
    if(crc!=h.hdrcrc32)RTERROR("header crc failure");
    fileSize=f.Size();
    lastRootOffset=h.lastRootOffset;
    f.Seek(lastRootOffset);
    lastRoot.Read(f);
    rootSize=lastRoot.size;
    opened=true;
    readOnly=readonly;
  }
  void Create(const char* file,int rsz=1024,bool cached=false)
  {
    if(File::Exists(file))RTERROR("chunk file already exists");
    rootSize=rsz;
    f.RWCreate(file);
    f.SetUnbuffered();
    isCached=cached;
    if(cached)f.OpenInMemory(ChunkStartItem::Size()*rootSize);
    FileHeader h;
    h.magic=_cf_magic;
    h.version=_cf_version;
    h.flags=0;
    h.lastRootOffset=h.Size();
    h.Write(f);
    RootRecord r;
    r.size=rootSize;
    r.count=0;
    r.nextPage=0;
    lastRootOffset=f.Size();
    r.Write(f);
    lastRoot=r;
    ChunkStartItem it;
    /*
    memset(&it,0,sizeof(it));
    for(int i=0;i<rootSize;i++)
    {
      it.Write(f);
    }
    */
    f.ZeroFill(it.Size()*rootSize);
    fileSize=f.Size();
    opened=true;
    readOnly=false;
  }

  void DiscardCache()
  {
    if(isCached)
    {
      f.DiscardCache();
    }
  }

  void Close()
  {
    if(!opened)return;
    if(!readOnly)
    {
      f.Seek(0);
      FileHeader h;
      h.Read(f);
      h.lastRootOffset=lastRootOffset;
      f.Seek(0);
      h.Write(f);
      if(isCached)f.MemoryFlush();
    }
    f.Close();
    opened=false;
  }

  void Flush(int maxSpeed=0)
  {
    if(isCached)f.MemoryFlush(maxSpeed);
  }

  File::offset_type Size()
  {
    return f.Size();
  }


#ifndef _WIN32
  friend class smsc::core::buffers::ChunkFile::ChunkHandle;
#endif

  class ChunkHandle{
  public:
    ChunkHandle(ChunkFile& _cf,ChunkId _id):cf(_cf),id(_id)
    {
      chunkReadOffset=0;
      readCount=0;
    }
    ~ChunkHandle()
    {
    }

    ChunkId GetId()
    {
      return id;
    }

    bool Read(R& r)
    {
      if(current==last && readCount==last.count)return false;
      cf.f.Seek(chunkReadOffset);
      r.Read(cf.f);
      readCount++;
      chunkReadOffset+=R::Size();
      if(readCount==current.count && current.nextPage!=0)
      {
        chunkReadOffset=current.nextPage;
        cf.f.Seek(current.nextPage);
        current.Read(cf.f);
        readCount=0;
        chunkReadOffset+=current.Size();
      }
      return true;
    }

    uint32_t ReadArray(R* arr,uint32_t count)
    {
      if(current==last && readCount==last.count)return 0;
      uint32_t rd=0;
      for(uint32_t i=0;i<count;i++)
      {
        if(!Read(*arr))return rd;
        arr++;
        rd++;
      }
      return rd;
    }

    void Write(const R& r)
    {
      if(cf.readOnly)RTERROR("attempt to write to read only chunk file");
      if(last.count==last.size)
      {
        ChunkPageHeader ph;
        File::offset_type off=cf.fileSize;
        cf.f.SeekEnd(0);
        ph.magic=_cf_ch_magic;
        ph.size=pageChunks;
        ph.count=0;
        ph.nextPage=0;
        ph.Write(cf.f);
        cf.fileSize+=ph.Size();
        cf.f.ZeroFill(pageChunks*R::Size());
        /*
        for(int i=0;i<pageChunks;i++)
        {
          R::WriteBadValue(cf.f);
        }
        */
        cf.fileSize+=pageChunks*R::Size();
        if(current==last)
        {
          current.nextPage=off;
        }
        if(first==last)
        {
          first.nextPage=off;
        }
        last.nextPage=off;
        cf.f.Seek(lastPageOffset);
        last.Write(cf.f);
        last=ph;
        ChunkStartItem it;
        cf.f.Seek(id);
        it.Read(cf.f);
        it.lastPage=off;
        cf.f.Seek(id);
        it.Write(cf.f);
        lastPageOffset=off;
      }
      cf.f.Seek(lastPageOffset+last.Size()+last.count*R::Size());
      r.Write(cf.f);
      if(first==last)first.count++;
      if(current==last)current.count++;
      last.count++;
      cf.f.Seek(lastPageOffset);
      last.Write(cf.f);
    }

    /*
    void WriteArray(const R* arr,uint32_t count)
    {
    }
    */

  private:
    ChunkHandle(const ChunkHandle&);
    void operator=(const ChunkHandle&);

    ChunkFile& cf;
    ChunkId    id;
    File::offset_type chunkReadOffset;
    uint32_t readCount;
    File::offset_type lastPageOffset;
    ChunkPageHeader first,current,last;
    int pageChunks;
    friend class ChunkFile;
  };

  ChunkHandle* CreateChunk(int pageChunks=1024)
  {
    if(readOnly)RTERROR("Attempt to create chunk in readonly chunk file");
    if(lastRoot.count==lastRoot.size)
    {
      File::offset_type newPageOffset=fileSize;
      lastRoot.nextPage=newPageOffset;
      RootRecord r;
      r.size=rootSize;
      r.count=0;
      r.nextPage=0;
      f.Seek(fileSize);
      r.Write(f);
      fileSize+=r.Size();

      ChunkStartItem it;
      /*
      it.firstPage=0;
      it.lastPage=0;
      for(uint32_t i=0;i<r.size;i++)
      {
        it.Write(f);
      }
      */
      f.ZeroFill(it.Size()*r.size);
      fileSize+=it.Size()*r.size;

      f.Seek(lastRootOffset);
      lastRoot.Write(f);
      lastRootOffset=newPageOffset;
      lastRoot=r;
    }
    ChunkStartItem it;
    ChunkId id=lastRootOffset+sizeof(lastRoot)+lastRoot.count*sizeof(it);
    it.firstPage=fileSize;
    it.lastPage=fileSize;
    ChunkPageHeader ph;
    ph.magic=_cf_ch_magic;
    ph.size=pageChunks;
    ph.count=0;
    ph.nextPage=0;
    f.SeekEnd(0);
    ph.Write(f);
    fileSize+=ph.Size();
    f.ZeroFill(pageChunks*R::Size());
    /*
    for(int i=0;i<pageChunks;i++)
    {
      R::WriteBadValue(f);
    }
    */
    fileSize+=pageChunks*R::Size();
    lastRoot.count++;

    f.Seek(id);
    it.Write(f);

    f.Seek(lastRootOffset);
    lastRoot.Write(f);

    ChunkHandle *ret=new ChunkHandle(*this,id);
    ret->first=ph;
    ret->current=ph;
    ret->last=ph;
    ret->lastPageOffset=it.lastPage;
    ret->pageChunks=pageChunks;

    return ret;
  }

  ChunkHandle* OpenChunk(ChunkId id)
  {
    std::auto_ptr<ChunkHandle> ret(new ChunkHandle(*this,id));
    f.Seek(id);
    ChunkStartItem it;
    it.Read(f);
    f.Seek(it.firstPage);
    ret->first.Read(f);
    if(it.firstPage==it.lastPage)
    {
      ret->last=ret->first;
    }else
    {
      f.Seek(it.lastPage);
      ret->last.Read(f);
    }
    ret->lastPageOffset=it.lastPage;
    ret->current=ret->first;
    ret->chunkReadOffset=it.firstPage+ret->first.Size();
    ret->pageChunks=ret->last.size;

    return ret.release();
  }

};

} //namespace buffers
} //namespace core
} //namespace smsc


#endif
