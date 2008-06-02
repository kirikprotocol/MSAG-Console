#include "CsvStore.hpp"
#include <algorithm>

namespace smsc{
namespace infosme{


static void MsgId2Info(uint64_t msgId,uint64_t& off,uint32_t& fk)
{
  off=msgId&0xffffffffull;
  fk=(uint32_t)((msgId>>32)&0xffffffffull);
}

static inline int hexfix(int val)
{
  return ((val/10)<<4)|(val%10);
}

CsvStore::~CsvStore()
{
  sync::MutexGuard mg(mtx);
  {
    DirMap::iterator end=dirs.end();
    for(DirMap::iterator it=dirs.begin();it!=end;it++)
    {
      delete it->second;
    }
    dirs.clear();
    end=pdirs.end();
    for(DirMap::iterator it=pdirs.begin();it!=end;it++)
    {
      delete it->second;
    }
    pdirs.clear();
  }
  OpenedFilesMap::iterator end=ofMap.end();
  for(OpenedFilesMap::iterator it=ofMap.begin();it!=end;it++)
  {
    delete it->second;
  }
  ofMap.clear();
}


void CsvStore::Init()
{
  typedef std::vector<std::string> StrVector;
  StrVector dirNames;
  using namespace smsc::core::buffers;
  File::ReadDir(location.c_str(),dirNames,File::rdfDirsOnly|File::rdfNoDots);
  //std::sort(dirNames.begin(),dirNames.end());
  for(StrVector::iterator it=dirNames.begin();it!=dirNames.end();it++)
  {
    Directory* dir=new Directory;
    if(sscanf(it->c_str(),"%x",&dir->date)!=1)
    {
      smsc_log_warn(log,"Unrecognized directory detected:%s",it->c_str());
      unknownDirs=true;
      continue;
    }
    dir->dirPath=location+*it;
    StrVector files;
    File::ReadDir(dir->dirPath.c_str(),files,File::rdfFilesOnly|File::rdfNoDots);
    std::sort(files.begin(),files.end());
    for(StrVector::iterator fit=files.begin();fit!=files.end();fit++)
    {
      int hour;
      if(sscanf(fit->c_str(),"%02x",&hour)!=1 || fit->find(".csv")==std::string::npos)
      {
        smsc_log_warn(log,"Unrecognized file detected:%s",fit->c_str());
        dir->unknownFiles=true;
        continue;
      }
      if(fit->find("processed")!=std::string::npos)
      {
        dir->arcFiles.push_back(*fit);
      }else
      {
        dir->files.push_back(*fit);
      }
    }
    if(dir->files.empty())
    {
      pdirs.insert(DirMap::value_type(dir->dirPath,dir));
    }else
    {
      dirs.insert(DirMap::value_type(dir->dirPath,dir));
    }
  }
}


uint32_t CsvStore::Delete(bool onlynew)
{
  DirMap* dd[2]={&dirs,&pdirs};
  uint32_t cnt=0;
  for(int i=0;i<2;i++)
  {
    for(DirMap::iterator dit=dd[i]->begin();dit!=dd[i]->end();dit++)
    {
      Directory& dir=*dit->second;
      for(StrList::iterator fit=dir.files.begin();fit!=dir.files.end();fit++)
      {
        buf::File f;
        f.ROpen((dir.dirPath+'/'+*fit).c_str());
        std::string str;
        while(f.ReadLine(str))
        {
          cnt++;
        }
        cnt--;
        f.Close();
        buf::File::Unlink((dir.dirPath+'/'+*fit).c_str());
      }
      dir.files.clear();
      if(!onlynew)
      {
        for(StrList::iterator fit=dir.arcFiles.begin();fit!=dir.arcFiles.end();fit++)
        {
          buf::File f;
          f.ROpen((dir.dirPath+'/'+*fit).c_str());
          std::string str;
          while(f.ReadLine(str))
          {
            cnt++;
          }
          cnt--;
          f.Close();
          buf::File::Unlink((dir.dirPath+'/'+*fit).c_str());
        }
        dir.arcFiles.clear();
      }
      if(dir.files.empty() && dir.arcFiles.empty() && !dir.unknownFiles)
      {
        buf::File::RmDir(dir.dirPath.c_str());
      }
    }
    if(!onlynew && !unknownDirs)
    {
      buf::File::RmDir(location.c_str());
    }
  }
  return cnt;
}

bool CsvStore::getNextMessage(Message &message)
{
  sync::MutexGuard mg(mtx);
  time_t now=time(NULL);
  for(;;)
  {
    uint8_t state;
    uint64_t off=currentReadFile?currentReadFile->ReadRecord(state,message):0;
    if(off!=0 && message.date>now)
    {
      currentReadFile->readOff=off;
      return false;
    }

    if(off==0)
    {
      if(currentReadFile)
      {
        currentReadFile->readAll=true;
      }
      for(;;)
      {
        if(dirs.empty())
        {
          return false;
        }
        Directory& dir=*dirs.begin()->second;
        if(dir.files.empty())
        {
          pdirs.insert(DirMap::value_type(dirs.begin()->first,dirs.begin()->second));
          dirs.erase(dirs.begin());
          continue;
        }
        std::string f=dir.files.front();
        dir.files.erase(dir.files.begin());
        dir.opened.push_back(f);

        uint32_t fk=dir.date;
        fk<<=8;
        int hour;
        sscanf(f.c_str(),"%02x",&hour);
        fk|=hour;

        OpenedFilesMap::iterator it=ofMap.find(fk);
        if(it!=ofMap.end())
        {
          currentReadFile=it->second;
          break;
        }

        currentReadFile=new CsvFile();
        currentReadFile->date=dir.date;
        currentReadFile->hour=hour;
        std::string fullPath=dir.dirPath+'/'+f;
        if(!currentReadFile->Open(fullPath.c_str(),false))
        {
          delete currentReadFile;
          currentReadFile=0;
          continue;
        }
        ofMap.insert(OpenedFilesMap::value_type(mkFileKey(currentReadFile),currentReadFile));
        break;
      }
      continue;
    }
    if(state!=NEW && state!=WAIT/* && state!=ENROUTE*/)
    {
      continue;
    }
    currentReadFile->openMessages.insert(off);
    currentReadFile->setState(off,WAIT);
    return true;
  }
}

void CsvStore::setMsgState(uint64_t msgId, uint8_t state)
{
  sync::MutexGuard mg(mtx);
  smsc_log_debug(log,"set state=%d for msgId=%lld",state,msgId);
  uint64_t off;
  uint32_t fk;
  MsgId2Info(msgId,off,fk);
  OpenedFilesMap::iterator it=ofMap.find(fk);
  if(it!=ofMap.end())
  {
    smsc_log_debug(log,"Found file in opened files");
    CsvFile& f=*it->second;
    if(f.setState(off,state))
    {
      closeFile(fk);
    }
    return;
  }
  uint32_t date=(uint32_t)((fk>>8)&0xffffff);
  uint32_t hour=(uint32_t)(fk&0xff);
  char filePath[64];
  sprintf(filePath,"%06x/%02x.csv",date,hour);
  std::string fullPath=location+filePath;
  CsvFile* fptr=new CsvFile;
  fptr->date=date;
  fptr->hour=hour;
  if(!fptr->Open(fullPath.c_str(),false))
  {
    throw smsc::util::Exception("File not found:'%s'",fullPath.c_str());
  }
  ofMap.insert(OpenedFilesMap::value_type(fk,fptr));
  if(fptr->setState(off,state))
  {
    closeFile(fk);
  }
}

void CsvStore::loadMessage(uint64_t msgId, Message &message, uint8_t &state)
{
  sync::MutexGuard mg(mtx);
  uint64_t off;
  uint32_t fk;
  MsgId2Info(msgId,off,fk);

  OpenedFilesMap::iterator it=ofMap.find(fk);
  if(it!=ofMap.end())
  {
    smsc_log_debug(log,"Found file in opened files");
    CsvFile& f=*it->second;
    buf::File::offset_type saveOff=f.readOff;
    f.readOff=off;
    try
    {
      f.ReadRecord(state,message);
    } catch(std::exception& e)
    {
      f.readOff=saveOff;
      throw;
    }
    f.readOff=saveOff;
    return;
  }

  CsvFile *fptr=new CsvFile;


  uint32_t date=(uint32_t)((fk>>8)&0xffffff);
  uint32_t hour=(uint32_t)(fk&0xff);

  char buf[64];
  sprintf(buf,"%06x/%02x.csv",date,hour);
  std::string fullPath=location;
  fullPath+=buf;
  fptr->date=date;
  fptr->hour=hour;
  if(!fptr->Open(fullPath.c_str(),false))throw smsc::util::Exception("MsgId=%lld requested. Failed to open file %s",msgId,buf);
  fptr->readOff=off;
  fptr->ReadRecord(state,message);
  ofMap.insert(OpenedFilesMap::value_type(fk,fptr));
}


void CsvStore::createMessage(time_t date,const Message& message,uint8_t state)
{
  sync::MutexGuard mg(mtx);
  struct tm t;
  localtime_r(&date,&t);
  int year=t.tm_year%100;
  int mon=t.tm_mon+1;
  int day=t.tm_mday;
  int hour=t.tm_hour;
  uint32_t xdate=hexfix(year)<<8;
  xdate|=hexfix(mon);
  xdate<<=8;
  xdate|=hexfix(day);
  hour=hexfix(hour);
  smsc_log_debug(log,"create msg for date %04d.%02d.%02d/%02d -> %06x/%02x",t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,xdate,hour);

  uint32_t fk=xdate;
  fk<<=8;
  fk|=hour;

  OpenedFilesMap::iterator it=ofMap.find(fk);
  if(it!=ofMap.end())
  {
    smsc_log_debug(log,"reusing opened file");
    it->second->AppendRecord(NEW,date,message);
    return;
  }

  char dirName[64];
  sprintf(dirName,"%06x",xdate);
  std::string dirPath=location;
  dirPath+=dirName;
  if(!buf::File::Exists(dirPath.c_str()))
  {
    buf::File::MkDir(dirPath.c_str(),0755);
    Directory* dir=new Directory;
    dir->date=xdate;
    dir->dirPath=dirPath;
    dirs.insert(DirMap::value_type(dirPath,dir));
  }
  CsvFile* f=new CsvFile;
  f->date=xdate;
  f->hour=hour;
  char fileName[64];
  sprintf(fileName,"%02x.csv",hour);
  std::string fullPath=dirPath;
  fullPath+='/';
  fullPath+=fileName;
  if(!buf::File::Exists(fullPath.c_str()))
  {
    DirMap::iterator dit=dirs.find(dirPath);
    if(dit==dirs.end())
    {
      dit=pdirs.find(dirPath);
      if(dit==pdirs.end())
      {
        throw smsc::util::Exception("Dir %s not found.",dirPath.c_str());
      }
      Directory* dir=dit->second;
      pdirs.erase(dit);
      dit=dirs.insert(DirMap::value_type(dirPath,dir)).first;
    }
    dit->second->files.push_back(fileName);
  }
  f->Open(fullPath.c_str());
  f->AppendRecord(state,date,message);
  ofMap.insert(OpenedFilesMap::value_type(fk,f));
  smsc_log_debug(log,"opened new file:%s",fullPath.c_str());
}


bool CsvStore::enrouteMessage(uint64_t msgId)
{
  sync::MutexGuard mg(mtx);
  uint64_t off;
  uint32_t fk;
  MsgId2Info(msgId,off,fk);
  OpenedFilesMap::iterator it=ofMap.find(fk);
  if(it!=ofMap.end())
  {
    int st=it->second->getState(off);
    if(st!=WAIT)
    {
      return false;
    }
    if(it->second->setState(off,ENROUTE))
    {
      closeFile(fk);
    }
    return true;
  }
  char fileName[64];
  uint32_t date=fk>>8;
  uint32_t hour=fk&0xff;
  sprintf(fileName,"%06x/%02x.csv",date,hour);
  CsvFile* fptr=new CsvFile;
  fptr->date=date;
  fptr->hour=hour;
  std::string fullPath=location+fileName;
  if(!fptr->Open(fullPath.c_str(),false))
  {
    return false;
  }
  if(fptr->getState(off)!=WAIT)
  {
    return false;
  }
  ofMap.insert(OpenedFilesMap::value_type(fk,fptr));
  if(fptr->setState(off,ENROUTE))
  {
    closeFile(fk);
  }
  return true;
}

void CsvStore::finalizeMsg(uint64_t msgId, time_t fdate, uint8_t state)
{
  sync::MutexGuard mg(mtx);
  uint64_t off;
  uint32_t fk;
  MsgId2Info(msgId,off,fk);
  OpenedFilesMap::iterator it=ofMap.find(fk);
  if(it!=ofMap.end())
  {
    if(it->second->setStateAndDate(off,state,fdate))
    {
      closeFile(fk);
    }
    return;
  }
  char fileName[64];
  uint32_t date=fk>>8;
  uint32_t hour=fk&0xff;
  sprintf(fileName,"%06x/%02x.csv",date,hour);
  std::string fullPath=location+fileName;
  CsvFile* fptr=new CsvFile;
  fptr->date=date;
  fptr->hour=hour;
  if(!fptr->Open(fullPath.c_str(),false))
  {
    throw smsc::util::Exception("File not found:'%s'",fullPath.c_str());
  }
  ofMap.insert(OpenedFilesMap::value_type(fk,fptr));
  if(fptr->setStateAndDate(off,state,fdate))
  {
    closeFile(fk);
  }
}

void CsvStore::closeFile(uint32_t fk)
{
  OpenedFilesMap::iterator it=ofMap.find(fk);
  if(it==ofMap.end())
  {
    smsc_log_warn(log,"failed to find file with fk=%x to close",fk);
    return;
  }
  CsvFile* fptr=it->second;
  ofMap.erase(it);

  std::string of=fptr->f.getFileName();
  size_t pos=of.rfind('.');
  of.erase(pos);
  of+="processed.csv";
  fptr->f.Rename(of.c_str());
  delete fptr;
}



bool CsvStore::CsvFile::Open(const char *argFileName,bool cancreate)
{
  fileName=argFileName;
  if(buf::File::Exists(argFileName))
  {
    std::string ln;
    f.RWOpen(argFileName);
    fileSize=f.Size();
    f.ReadLine(ln);//skip header
    readOff=f.Pos();
    buf::File::offset_type off=readOff;
    while(f.ReadLine(ln))
    {
      if(ln.length() && ln[0]==ENROUTE)
      {
        openMessages.insert(off);
      }
      off=f.Pos();
    }
  }else
  {
    if(!cancreate)
    {
      return false;
    }
    f.RWCreate(argFileName);
    const char* header="STATE,DATE,ABONENT,REGION,MESSAGE\n";
    f.Write(header,strlen(header));
    fileSize=f.Size();
    readOff=fileSize;
  }
  return true;
}

uint64_t CsvStore::CsvFile::ReadRecord(uint8_t &state,Message& message)
{
  f.Seek(readOff);
  uint64_t off=readOff;
  if(off==fileSize)
  {
    return 0;
  }
  state=f.ReadByte()-'0';
  if(f.ReadByte()!=',')
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",fileName.c_str(),f.Pos()-1);
  }
  char dateBuf[16]={0,};
  f.Read(dateBuf,12);
  struct tm t;
  memset(&t,0,sizeof(t));
  sscanf(dateBuf,"%02d%02d%02d%02d%02d%02d",&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec);
  t.tm_year+=100;
  t.tm_mon--;
  t.tm_isdst=-1;
  message.date=mktime(&t);

  if(f.ReadByte()!=',')
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",fileName.c_str(),f.Pos()-1);
  }

  message.abonent="";
  uint8_t c;
  while((c=f.ReadByte())!=',')
  {
    message.abonent+=(char)c;
  }

  message.regionId="";
  while((c=f.ReadByte())!=',')
  {
    message.regionId+=(char)c;
  }

  message.message="";
  if(f.ReadByte()!='"')
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",fileName.c_str(),f.Pos()-1);
  }
  while((c=f.ReadByte())!='"')
  {
    if(c=='\\')
    {
      c=f.ReadByte();
      if(c=='n')
      {
        message.message+="\n";
      }else
      {
        message.message+=(char)c;
      }
    }else
    {
      message.message+=(char)c;
    }
  }
  c=f.ReadByte();
  if(c==0x0d)
  {
    if(f.ReadByte()!=0x0a)
    {
      throw smsc::util::Exception("Corrupted store file:'%s' at %lld",fileName.c_str(),f.Pos()-1);
    }
  }else if(c!=0x0a)
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",fileName.c_str(),f.Pos()-1);
  }

  message.id=date;
  message.id<<=8;
  message.id|=hour;
  message.id<<=32;
  message.id|=off;

  readOff=f.Pos();
  return off;
}


bool CsvStore::CsvFile::setState(uint64_t off, uint8_t state)
{
  bool rv=false;
  if(state>ENROUTE)
  {
    static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("csvfile");
    smsc_log_debug(log,"setState(%lld,%d). openedMessages=%d",off,state,openMessages.size());
    OffSet::iterator mit=openMessages.find(off);
    if(mit!=openMessages.end())
    {
      openMessages.erase(mit);
      smsc_log_debug(log,"openedMessage erased");
      if(readAll && openMessages.empty())
      {
        rv=true;
      }
    }
  }
  f.Seek(off);
  f.WriteByte(state+'0');
  f.Flush();
  return rv;
}

static std::string escapeMessage(const char* msg)
{
  std::string rv;
  const char* ppos=msg;
  while(*msg)
  {
    if(*msg=='\\' || *msg=='"' || *msg==0x0a)
    {
      rv.append(ppos,msg-ppos);
      rv+="\\";
      if(*msg==0x0a)
      {
        rv+='n';
      }else
      {
        rv+=*msg;
      }
      ppos=msg+1;
    }
    msg++;
  }
  rv.append(ppos,msg-ppos);
  return rv;
}

void CsvStore::CsvFile::AppendRecord(uint8_t state,time_t fdate,const Message& message)
{
  char timestamp[16];// YYMMDDhhmmss
  struct tm t;
  localtime_r(&fdate,&t);
  sprintf(timestamp,"%02d%02d%02d%02d%02d%02d",t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
  std::string escapedMsg=escapeMessage(message.message.c_str());
  f.SeekEnd(0);
  f.WriteByte('0'+state);
  f.WriteByte(',');
  f.Write(timestamp,12);
  f.WriteByte(',');
  f.Write(message.abonent.c_str(),message.abonent.length());
  f.WriteByte(',');
  f.Write(message.regionId.c_str(),message.regionId.length());
  f.WriteByte(',');
  f.WriteByte('"');
  f.Write(escapedMsg.c_str(),escapedMsg.length());
  f.WriteByte('"');
  f.WriteByte('\n');
  f.Flush();
  fileSize=f.Size();
  readAll=false;
}

uint8_t CsvStore::CsvFile::getState(uint64_t off)
{
  f.Seek(off);
  return f.ReadByte()-'0';
}

bool CsvStore::CsvFile::setStateAndDate(uint64_t off,uint8_t state, time_t fdate)
{
  bool rv=false;
  if(state>ENROUTE)
  {
    static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("csvfile");
    smsc_log_debug(log,"setState(%lld,%d). openedMessages=%d",off,state,openMessages.size());
    OffSet::iterator mit=openMessages.find(off);
    if(mit!=openMessages.end())
    {
      openMessages.erase(mit);
      smsc_log_debug(log,"openedMessage erased");
      if(readAll && openMessages.empty())
      {
        rv=true;
      }
    }
  }
  f.Seek(off);
  f.WriteByte(state+'0');
  f.SeekCur(1);
  struct tm t;
  localtime_r(&fdate,&t);
  char tsbuf[32];
  sprintf(tsbuf,"%02d%02d%02d%02d%02d%02d",t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
  f.Write(tsbuf,12);
  f.Flush();
  return rv;
}

static CsvStore::StrList::iterator GetEnd(CsvStore::Directory& dir,int idx)
{
  if(idx==0)
  {
    return dir.files.end();
  }else if(idx==1)
  {
    return dir.opened.end();
  }else
  {
    return dir.arcFiles.end();
  }
}

bool CsvStore::FullScan::Next(uint8_t &state, Message &msg)
{
  for(;;)
  {
    if(first)
    {
      if(didx==0 && store.dirs.empty())
      {
        didx++;
        continue;
      }
      if(didx==1 && store.pdirs.empty())
      {
        return false;
      }
      dit=didx==0?store.dirs.begin():store.pdirs.begin();
      fit=fidx==0?dit->second->files.begin():fidx==1?dit->second->opened.begin():dit->second->arcFiles.begin();
      if(fit==GetEnd(*dit->second,fidx))
      {
        fidx++;
        if(fidx==3)
        {
          didx++;
          if(didx==2)
          {
            return false;
          }
          continue;
        }
        continue;
      }
      first=false;
      f.Open(fit->c_str());
    }
    Directory& dir=*dit->second;
    uint64_t off=f.ReadRecord(state,msg);
    if(off==0)
    {
      fit++;
      while(fit==GetEnd(dir,fidx))
      {
        fidx++;
        if(fidx==3)
        {
          didx++;
          if(didx==2)
          {
            return false;
          }
          fidx=0;
          first=true;
          return Next(state,msg);
        }
        fit=fidx==0?dit->second->files.begin():fidx==1?dit->second->opened.begin():dit->second->arcFiles.begin();
      }
      f.Open(fit->c_str());
      continue;
    }
    return true;
  }
  return false;
}


}
}



