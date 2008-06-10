#include "CsvStore.hpp"
#include <memory>
#include <algorithm>

namespace smsc{
namespace infosme{


static void MsgId2Info(uint64_t msgId,uint64_t& off,int& date,int& hour)
{
  off=msgId&0xffffffffull;
  uint32_t fk=(uint32_t)((msgId>>32)&0xffffffffull);
  date=fk>>8;
  hour=fk&0xff;
}

static inline int hexfix(int val)
{
  return ((val/10)<<4)|(val%10);
}

CsvStore::~CsvStore()
{
  sync::MutexGuard mg(mtx);
  DirMap::iterator end=dirs.end();
  for(DirMap::iterator it=dirs.begin();it!=end;it++)
  {
    delete it->second;
  }
  dirs.clear();
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
    dirs.insert(DirMap::value_type(dir->date,dir));
    StrVector files;
    File::ReadDir(dir->dirPath.c_str(),files,File::rdfFilesOnly|File::rdfNoDots);
    for(StrVector::iterator fit=files.begin();fit!=files.end();fit++)
    {
      int hour;
      if(sscanf(fit->c_str(),"%02x",&hour)!=1 || fit->find(".csv")==std::string::npos)
      {
        smsc_log_warn(log,"Unrecognized file detected:%s",fit->c_str());
        dir->unknownFiles=true;
        continue;
      }
      CsvFile* f=new CsvFile(dir->date,hour,dir);
      if(fit->find("processed")!=std::string::npos)
      {
        f->processed=true;
      }
      dir->files.insert(FileMap::value_type(hour,f));
    }
  }
  curDir=dirs.begin();
  if(curDir!=dirs.end())
  {
    curFile=curDir->second->files.begin();
  }
}


uint32_t CsvStore::Delete(bool onlynew)
{
  uint32_t cnt=0;
  DirMap::iterator dend=dirs.end();
  for(DirMap::iterator dit=dirs.begin();dit!=dend;dit++)
  {
    Directory& dir=*dit->second;
    FileMap::iterator fend=dir.files.end();
    for(FileMap::iterator fit=dir.files.begin();fit!=fend;fit++)
    {
      buf::File f;
      std::string fullPath=fit->second->fullPath();
      f.ROpen(fullPath.c_str());
      std::string str;
      while(f.ReadLine(str))
      {
        cnt++;
      }
      cnt--;
      f.Close();
      buf::File::Unlink(fullPath.c_str());
      delete fit->second;
    }
    dir.files.clear();
    if(!dir.unknownFiles)
    {
      buf::File::RmDir(dir.dirPath.c_str());
    }
  }
  if(!unknownDirs)
  {
    buf::File::RmDir(location.c_str());
  }
  return cnt;
}

void CsvStore::closeAllFiles()
{
  sync::MutexGuard mg(mtx);
  for(DirMap::iterator dit=dirs.begin();dit!=dirs.end();dit++)
  {
    Directory& dir=*dit->second;
    for(FileMap::iterator fit=dir.files.begin();fit!=dir.files.end();fit++)
    {
      fit->second->Close(false);
    }
  }
  closeSet.clear();
  curDir=dirs.begin();
  if(curDir!=dirs.end())
  {
    curFile=curDir->second->files.begin();
  }
}


bool CsvStore::getNextMessage(Message &message)
{
  sync::MutexGuard mg(mtx);
  time_t now=time(NULL);
  if(!closeSet.empty())//some files are ready to be closed
  {
    struct tm t;
    localtime_r(&now,&t);
    typedef std::vector<CloseSet::iterator> KillVector;
    KillVector tokill;
    for(CloseSet::iterator it=closeSet.begin();it!=closeSet.end();it++)
    {
      int hour=(*it)&0xff;
      hour=((hour&0xf0)>>4)*10+(hour&0x0f);
      if(t.tm_hour!=hour)//check if current to file hour is already passed
      {
        smsc_log_debug(log,"Try to close file:%x(hour=%d, curHour=%d",*it,hour,t.tm_hour);
        uint64_t msgId=*it;
        msgId<<=32;
        uint64_t off;
        CsvFile& f=findFile(__func__,msgId,off);
        f.Close();
        tokill.push_back(it);
      }
    }
    for(KillVector::iterator it=tokill.begin();it!=tokill.end();it++)
    {
      closeSet.erase(*it);
    }
  }
  if(curDir==dirs.end())//all dirs and files are processed
  {
    return false;
  }
  while(curFile==curDir->second->files.end())
  {
    curDir++;
    if(curDir==dirs.end())
    {
      return false;
    }
    curFile=curDir->second->files.begin();
  }
  for(;;)
  {
    if(!curFile->second->isOpened())
    {
      curFile->second->Open(false);
    }
    CsvFile::Record rec;
    CsvFile::GetRecordResult res=curFile->second->getNextRecord(rec,now);
    if(res!=CsvFile::grrNoMoreMessages)
    {
      if(res==CsvFile::grrRecordNotReady)
      {
        smsc_log_debug(log,"date of next message is in future (now=%ld, date=%ld)",now,rec.msg.date);
        return false;
      }
      message=rec.msg;
      curFile->second->setState(message.id,WAIT);
      return true;
    }
    if(curFile->second->readAll && curFile->second->openMessages==0)
    {
      canClose(*curFile->second);
    }

    smsc_log_debug(log,"finished reading current file '%s' (%s,%d)",curFile->second->fullPath().c_str(),curFile->second->readAll?"readall":"not readall",curFile->second->openMessages);
    curFile++;
    while(curFile==curDir->second->files.end())
    {
      curDir++;
      if(curDir==dirs.end())
      {
        smsc_log_debug(log,"There are no dirs/files left to read from");
        return false;
      }
      curFile=curDir->second->files.begin();
    }
    curFile->second->Open();
    continue;
  }
}

//extract from msgId date and hour and find corresponding CsvFile object.
//extracted from msgId msg offset returned
CsvStore::CsvFile& CsvStore::findFile(const char* func,uint64_t msgId, uint64_t &off)
{
  int date,hour;
  MsgId2Info(msgId,off,date,hour);
  DirMap::iterator dit=dirs.find(date);
  if(dit==dirs.end())
  {
    throw smsc::util::Exception("%s:Directory not found:'%06x'",func,date);
  }
  Directory& dir=*dit->second;
  FileMap::iterator fit=dir.files.find(hour);
  if(fit==dir.files.end())
  {
    throw smsc::util::Exception("%s:File not found:'%06x/%02x'",func,date,hour);
  }
  return *fit->second;
}

//mark file as 'ready to close'
//will be closed in getNextMessage after hour check
void CsvStore::canClose(CsvFile &file)
{
  uint32_t fk=file.date;
  fk<<=8;
  fk|=file.hour;
  smsc_log_debug(log,"Mark as 'canClose' file %x",fk);
  closeSet.insert(fk);
}

//message was added to file that was ready to be closed.
//remove it from corresponding set
void CsvStore::removeCanClose(CsvFile &file)
{
  uint32_t fk=file.date;
  fk<<=8;
  fk|=file.hour;
  CloseSet::iterator it=closeSet.find(fk);
  if(it!=closeSet.end())
  {
    smsc_log_debug(log,"Remove 'canClose' mark from file %x",fk);
    closeSet.erase(it);
  }
}


void CsvStore::setMsgState(uint64_t msgId, uint8_t state)
{
  sync::MutexGuard mg(mtx);
  smsc_log_debug(log,"setMsgState: state=%d msgId=#%llx",state,msgId);
  uint64_t off;
  CsvFile& file=findFile(__func__,msgId,off);
  if(file.processed)
  {
    //can only delete messages from processed files.
    //any other states have no meaning anyway
    if(state!=DELETED)
    {
      throw smsc::util::Exception("%s:Unexpected state %d for msgId=#%llx",__func__,state,msgId);
    }
    File f;
    f.WOpen(file.fullPath().c_str());
    f.Seek(off);
    f.WriteByte('0'+state);
    return;
  }
  if(!file.isOpened())
  {
    file.Open();
  }
  file.setState(msgId,state);
  //all records are read and all messages are in final state
  //file can be closed
  if(file.readAll && file.openMessages==0)
  {
    canClose(file);
  }
}

void CsvStore::loadMessage(uint64_t msgId, Message &message, uint8_t &state)
{
  sync::MutexGuard mg(mtx);
  uint64_t off;
  CsvFile& file=findFile(__func__,msgId,off);
  if(!file.isOpened())//if file is not yet opened, just load single message, not whole file.
  {
    file.f.ROpen(file.fullPath().c_str());
    file.f.Seek(off);
    CsvFile::Record rec;
    file.ReadRecord(rec);
    message=rec.msg;
    state=rec.state;
    file.f.Close();
    return;
  }
  CsvFile::Record& rec=file.findRecord(msgId);
  state=rec.state;
  message=rec.msg;
}


uint64_t CsvStore::createMessage(time_t date,const Message& message,uint8_t state)
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

  char dirName[64];
  sprintf(dirName,"%06x",xdate);
  std::string dirPath=location;
  dirPath+=dirName;
  Directory* dir;
  if(!buf::File::Exists(dirPath.c_str()))
  {
    smsc_log_debug(log,"%s:creating new dir:'%s'",__func__,dirPath.c_str());
    buf::File::MkDir(dirPath.c_str(),0755);
    dir=new Directory;
    dir->date=xdate;
    dir->dirPath=dirPath;
    DirMap::iterator dit=dirs.insert(DirMap::value_type(xdate,dir)).first;
  }else
  {
    smsc_log_debug(log,"%s:creating in existing dir '%s'",__func__,dirPath.c_str());
    DirMap::iterator dit=dirs.find(xdate);
    if(dit==dirs.end())
    {
      throw smsc::util::Exception("%s:Directory not found:'%s'",__func__,dirPath.c_str());
    }
    dir=dit->second;
  }

  FileMap::iterator fit=dir->files.find(hour);
  CsvFile* fptr;
  if(fit==dir->files.end())
  {
    fptr=new CsvFile(xdate,hour,dir);
    fptr->Open(true);
    fit=dir->files.insert(FileMap::value_type(hour,fptr)).first;
    smsc_log_debug(log,"%s:opened new file:'%s'",__func__,fptr->fullPath().c_str());
  }else
  {
    fptr=fit->second;
    if(!fptr->isOpened())
    {
      fptr->Open();
    }

  }
  if(curDir==dirs.end())
  {
    curDir=dirs.find(xdate);//always succeed, see above
    curFile=fit;
  }

  removeCanClose(*fptr);
  return fptr->AppendRecord(state,date,message);
}


void CsvStore::enrouteMessage(uint64_t msgId)
{
  sync::MutexGuard mg(mtx);
  uint64_t off;
  CsvFile& file=findFile(__func__,msgId,off);
  if(!file.isOpened())
  {
    file.Open();
  }
  file.setState(msgId,ENROUTE);
}

void CsvStore::finalizeMsg(uint64_t msgId, time_t fdate, uint8_t state)
{
  sync::MutexGuard mg(mtx);
  uint64_t off;
  CsvFile& file=findFile(__func__,msgId,off);
  if(!file.isOpened())
  {
    file.Open();
  }
  file.setStateAndDate(msgId,state,fdate);
  if(file.readAll && file.openMessages==0)
  {
    canClose(file);
  }
}

bool CsvStore::CsvFile::Open(bool cancreate)
{
  if(isOpened())
  {
    return true;
  }
  std::string fileName=fullPath();
  if(buf::File::Exists(fileName.c_str()))
  {
    std::string ln;
    f.RWOpen(fileName.c_str());
    buf::File::offset_type fileSize=f.Size();
    f.ReadLine(ln);//skip header
    bool haveNonFinal=false;
    Record rec;
    while(f.Pos()<fileSize)
    {
      ReadRecord(rec);
      if(rec.state==DELETED)
      {
        continue;
      }
      msgMap.insert(MessageMap::value_type(rec.msg.id,rec));
      if(rec.state<DELIVERED)//calculate number of messages in non-final state
      {
        openMessages++;
        haveNonFinal=true;
      }
    }
    readAll=!haveNonFinal;
    curMsg=msgMap.begin();
  }else
  {
    if(!cancreate)
    {
      return false;
    }
    f.RWCreate(fileName.c_str());
    const char* header="STATE,DATE,ABONENT,REGION,MESSAGE\n";
    f.Write(header,strlen(header));
  }
  return true;
}

void CsvStore::CsvFile::Close(bool argProcessed)
{
  if(!f.isOpened())
  {
    return;
  }
  if(!processed && argProcessed)
  {
    std::string fn=fullPath();
    size_t pos=fn.rfind('.');
    fn.erase(pos);
    fn+="processed.csv";
    f.Rename(fn.c_str());
  }
  processed=argProcessed;
  f.Close();
  msgMap.clear();
}

CsvStore::CsvFile::Record& CsvStore::CsvFile::findRecord(uint64_t msgId)
{
  MessageMap::iterator it=msgMap.find(msgId);
  if(it==msgMap.end())
  {
    throw smsc::util::Exception("Message #%llx not found in %s",msgId,fullPath().c_str());
  }
  return it->second;
}


CsvStore::CsvFile::GetRecordResult CsvStore::CsvFile::getNextRecord(CsvStore::CsvFile::Record& rec,time_t rdate,bool onlyNew)
{
  if(msgMap.empty() || curMsg==msgMap.end())
  {
    readAll=true;
    return grrNoMoreMessages;
  }
  if(rdate && curMsg->second.msg.date>rdate)
  {
    return grrRecordNotReady;
  }
  if(onlyNew)
  {
    while(curMsg->second.state>WAIT)
    {
      curMsg++;
      if(curMsg==msgMap.end())
      {
        readAll=true;
        return grrNoMoreMessages;
      }
    }
  }
  rec=curMsg->second;
  curMsg++;
  return grrRecordOk;
}


void CsvStore::CsvFile::ReadRecord(CsvStore::CsvFile::Record& rec)
{
  uint64_t off=f.Pos();
  ReadRecord(f,rec);
  rec.msg.id=date;
  rec.msg.id<<=8;
  rec.msg.id|=hour;
  rec.msg.id<<=32;
  rec.msg.id|=off;
}

void CsvStore::CsvFile::ReadRecord(buf::File &f, CsvStore::CsvFile::Record& rec)
{
  rec.state=f.ReadByte()-'0';
  if(f.ReadByte()!=',')
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",f.getFileName().c_str(),f.Pos()-1);
  }
  char dateBuf[16]={0,};
  f.Read(dateBuf,12);
  struct tm t;
  memset(&t,0,sizeof(t));
  sscanf(dateBuf,"%02d%02d%02d%02d%02d%02d",&t.tm_year,&t.tm_mon,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec);
  t.tm_year+=100;
  t.tm_mon--;
  t.tm_isdst=-1;
  rec.msg.date=mktime(&t);

  if(f.ReadByte()!=',')
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",f.getFileName().c_str(),f.Pos()-1);
  }

  rec.msg.abonent="";
  uint8_t c;
  while((c=f.ReadByte())!=',')
  {
    rec.msg.abonent+=(char)c;
  }

  rec.msg.regionId="";
  while((c=f.ReadByte())!=',')
  {
    rec.msg.regionId+=(char)c;
  }

  rec.msg.message="";
  if(f.ReadByte()!='"')
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",f.getFileName().c_str(),f.Pos()-1);
  }
  while((c=f.ReadByte())!='"')
  {
    if(c=='\\')
    {
      c=f.ReadByte();
      if(c=='n')
      {
        rec.msg.message+="\n";
      }else
      {
        rec.msg.message+=(char)c;
      }
    }else
    {
      rec.msg.message+=(char)c;
    }
  }
  c=f.ReadByte();
  if(c==0x0d)
  {
    if(f.ReadByte()!=0x0a)
    {
      throw smsc::util::Exception("Corrupted store file:'%s' at %lld",f.getFileName().c_str(),f.Pos()-1);
    }
  }else if(c!=0x0a)
  {
    throw smsc::util::Exception("Corrupted store file:'%s' at %lld",f.getFileName().c_str(),f.Pos()-1);
  }
}


void CsvStore::CsvFile::setState(uint64_t msgId, uint8_t state)
{
  Record& rec=findRecord(msgId);
  if(state>ENROUTE)
  {
    openMessages--;
  }
  rec.state=state;
  uint64_t off;
  int rdate,rhour;
  MsgId2Info(msgId,off,rdate,rhour);
  f.Seek(off);
  f.WriteByte(state+'0');
  f.Flush();
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

uint64_t CsvStore::CsvFile::AppendRecord(uint8_t state,time_t fdate,const Message& message)
{
  f.SeekEnd(0);
  uint64_t msgId=date;
  msgId<<=8;
  msgId|=hour;
  msgId<<=32;
  msgId|=f.Pos();
  Record rec;
  rec.state=state;
  rec.msg=message;
  rec.msg.date=fdate;
  rec.msg.id=msgId;
  MessageMap::iterator it=msgMap.insert(MessageMap::value_type(msgId,rec)).first;
  if(curMsg==msgMap.end())
  {
    curMsg=it;
  }
  char timestamp[16];// YYMMDDhhmmss
  struct tm t;
  localtime_r(&fdate,&t);
  sprintf(timestamp,"%02d%02d%02d%02d%02d%02d",t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
  std::string escapedMsg=escapeMessage(message.message.c_str());
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
  readAll=false;
  openMessages++;
  return msgId;
}

uint8_t CsvStore::CsvFile::getState(uint64_t msgId)
{
  Record& rec=findRecord(msgId);
  return rec.state;
}

void CsvStore::CsvFile::setStateAndDate(uint64_t msgId,uint8_t state, time_t fdate)
{
  Record& rec=findRecord(msgId);
  rec.state=state;
  int rdate,rhour;
  uint64_t off;
  MsgId2Info(msgId,off,rdate,rhour);
  f.Seek(off);
  f.WriteByte(state+'0');
  f.SeekCur(1);
  struct tm t;
  localtime_r(&fdate,&t);
  char tsbuf[32];
  sprintf(tsbuf,"%02d%02d%02d%02d%02d%02d",t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
  f.Write(tsbuf,12);
  f.Flush();
  if(state>ENROUTE)
  {
    openMessages--;
  }
}

bool CsvStore::FullScan::Next(uint8_t &state, Message &msg)
{
  sync::MutexGuard mg(store.mtx);
  for(;;)
  {
    if(dit==store.dirs.end())
    {
      return false;
    }
    while(fit==dit->second->files.end())
    {
      dit++;
      if(dit==store.dirs.end())
      {
        return false;
      }
      fit=dit->second->files.begin();
    }
    if(!f.isOpened())
    {
      f.hour=fit->second->hour;
      f.date=fit->second->date;
      f.dir=fit->second->dir;
      f.processed=fit->second->processed;
      f.Open();
    }
    CsvStore::CsvFile::Record rec;
    CsvStore::CsvFile::GetRecordResult res=f.getNextRecord(rec,0,false);
    if(res==CsvStore::CsvFile::grrNoMoreMessages)
    {
      f.Close(false);
      fit++;
      continue;
    }
    state=rec.state;
    msg=rec.msg;
    return true;
  }

  return false;
}


}
}



