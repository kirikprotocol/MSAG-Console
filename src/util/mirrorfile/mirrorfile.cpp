#include <stdlib.h>
#include <string>
#include <unistd.h>
#include "core/buffers/File.hpp"

namespace smsc{
namespace util{
namespace mirrorfile{

std::string prefix;
std::string curDir;
bool broken=false;

std::string makeMirrorFilename(const char* filename)
{
  std::string rv;
  if(filename && strlen(filename)>0 && filename[0]=='/')
  {
    rv=prefix+filename;
  }else
  {
    rv=prefix+curDir+filename;
  }
  return rv;
}

bool DirExists(const char* dir)
{
  struct stat st;
  if(stat(dir,&st)!=0)return false;
  return (st.st_mode&S_IFDIR)!=0;
}


bool MakePath(const std::string& path)
{
  int idx=0;
  std::string purePath=path.substr(0,path.rfind('/'));
  if(DirExists(purePath.c_str()))return true;
  for(;;)
  {
    idx=path.find('/',idx);
    if(idx==std::string::npos)return true;
    idx++;
    std::string subDir=path.substr(0,idx);
    if(DirExists(subDir.c_str()))continue;
    if(mkdir(subDir.c_str(),00755)!=0)return false;
  }
}

struct DupeFile:smsc::core::buffers::FileEventHandler{
  smsc::core::buffers::File dupe;
  DupeFile()
  {
    dupe.SetEventHandler(0);
  }
  void SetBroken(const char* e = NULL)
  {
    broken=true;
    fprintf(stderr,"Broken mirror file:%s : Error: %s",dupe.getFileName().c_str(), e ? e : "unknown");
  }
  virtual void onOpen(int mode,const char* fileName)
  {
    if(broken)return;
    //fprintf(stderr,"open:%s\n",fileName);
    std::string newFn;
    if(fileName && strlen(fileName)>0 && fileName[0]=='/')
    {
      newFn=prefix+fileName;
    }else
    {
      newFn=prefix+curDir+fileName;
    }
    MakePath(newFn);
    try{
      switch(mode)
      {
        case openRead:
        {
          dupe.ROpen(newFn.c_str());
        }break;
        case openWrite:
        {
          dupe.WOpen(newFn.c_str());
        }break;
        case openRW:
        {
          dupe.RWOpen(newFn.c_str());
        }break;
        case openCreate:
        {
          dupe.RWCreate(newFn.c_str());
        }break;
        case openAppend:
        {
          if(smsc::core::buffers::File::Exists(newFn.c_str()))
          {
            dupe.Append(newFn.c_str());
          }
          else
          {
            dupe.RWCreate(newFn.c_str());
          }
        }break;
      }
    }
  catch(std::exception& e)
    {
      SetBroken(e.what());
    }
  catch(...)
    {
      SetBroken();
    }
  }
  virtual void onRead(const void* data,size_t sz)
  {
    if(broken)return;
    try{
      dupe.SeekCur(sz);
    }catch(std::exception& e)
    {
      SetBroken(e.what());
    }catch(...)
    {
      SetBroken();
    }
  }
  virtual void onWrite(const void* data,size_t sz)
  {
    if(broken)return;
    try{
      dupe.Write(data,sz);
      dupe.Flush();
    }
    catch(std::exception& e)
    {
      SetBroken(e.what());
    }
    catch(...)
    {
      SetBroken();
    }
  }
  virtual void onSeek(int whence,int64_t offset)
  {
    if(broken)return;
    try{
      dupe.Seek(offset,whence);
    }
    catch(std::exception& e)
    {
      SetBroken(e.what());
    }
    catch(...)
    {
      SetBroken();
    }
  }
  virtual void onRename(const char* newname)
  {
    MakePath(makeMirrorFilename(newname).c_str());
    try{
      dupe.Rename(makeMirrorFilename(newname).c_str());
    }
    catch(std::exception& e)
    {
      SetBroken(e.what());
    }
    catch(...)
    {
      SetBroken();
    }
  }
};

struct DupeGlobalFileEventHandler:smsc::core::buffers::GlobalFileEventHandler{
  bool recursion;
  DupeGlobalFileEventHandler()
  {
    if(broken)return;
    const char* var=getenv("HS_MIRROR_PATH");
    if(var)
    {
      fprintf(stderr,"Mirroring enabled at %s\n",var);
      prefix=var;
      if(prefix.length()>0 && *prefix.rbegin()!='/')prefix+='/';
      char buf[1024]={0,};
      getcwd(buf,sizeof(buf));
      curDir=buf;
      if(curDir.length()>0 && *curDir.rbegin()!='/')curDir+='/';
      curDir.erase(0,1);
      smsc::core::buffers::GlobalFileEventHandler::setGlobalFileEventHandler(this);
    }
    recursion=false;
  }
  virtual void onCreateFileObject(smsc::core::buffers::File* f)
  {
    //fprintf(stderr,"onCreateFileObject\n");
    if(recursion || broken)return;
    recursion=true;
    f->SetEventHandler(new DupeFile());
    recursion=false;
  }
  virtual void onDestroyFileObject(smsc::core::buffers::File* f)
  {
    if(f->GetEventHandler())delete f->GetEventHandler();
  }
  virtual void onRename(const char* oldName,const char* newName)
  {
    if(broken)return;
    MakePath(makeMirrorFilename(newName).c_str());
    rename(makeMirrorFilename(oldName).c_str(),makeMirrorFilename(newName).c_str());
  }
  virtual void onUnlink(const char* fileName)
  {
    if(broken)return;
    unlink(makeMirrorFilename(fileName).c_str());
  }

  void DummyMethod(){}
};

DupeGlobalFileEventHandler globalFEH;

}
}
}

void InitMirrorFile()
{
  smsc::util::mirrorfile::globalFEH.DummyMethod();
}

