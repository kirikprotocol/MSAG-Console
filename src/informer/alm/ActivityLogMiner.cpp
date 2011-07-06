#include "ActivityLogMiner.hpp"
#include "informer/io/InfosmeException.h"
#include "core/buffers/File.hpp"
#include "informer/data/CommonSettings.h"

namespace eyeline{
namespace informer{
namespace alm{

ActivityLogMiner::ActivityLogMiner():reqIdSeq(0)
{
  requestTimeout=60*60;
  log=smsc::logger::Logger::getInstance("alm");
}

void ActivityLogMiner::init(const std::string& argPath,time_t argRequestTimeout)
{
  path=argPath;
  requestTimeout=argRequestTimeout;
}


int ActivityLogMiner::createRequest(dlvid_type dlvId,const ALMRequestFilter& filter)
{
  Request* req=new Request;
  req->ref();
  req->dlvId=dlvId;
  req->filter=filter;
  req->curDate=filter.startDate;
  req->offset=0;
  sync::MutexGuard mg(mtx);
  time_t now=time(0);
  while(!timeMap.empty() && timeMap.begin()->first<now)
  {
    TimeMap::iterator tit=timeMap.begin();
    ReqMap::iterator it=reqMap.find(tit->second);
    if(it!=reqMap.end())
    {
      it->second->unref();
      reqMap.erase(it);
    }
    timeMap.erase(tit);
  }
  int rv=reqIdSeq++;
  smsc_log_debug(log,"created request with id=%d, from %lld to %lld for delivery %d",rv,msgTimeToYmd(filter.startDate),msgTimeToYmd(filter.endDate),dlvId);
  req->timeIt=timeMap.insert(TimeMap::value_type(now+requestTimeout,rv));
  reqMap.insert(ReqMap::value_type(rv,req));
  return rv;
}

std::string ActivityLogMiner::mkFilePath(dlvid_type dlvId,const ::tm& tm)
{
  char buf[128];
  sprintf(makeDeliveryPath(buf,dlvId),
                      "activity_log/%04u.%02u.%02u/%02u/%02u.log",
                      tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                      tm.tm_hour, tm.tm_min );
  return path+buf;
}

std::string ActivityLogMiner::mkDatePath(dlvid_type dlvId,const ::tm& tm)
{
  char buf[128];
  sprintf(makeDeliveryPath(buf,dlvId),
                      "activity_log/%04u.%02u.%02u",
                      tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday
                      );
  return path+buf;
}

std::string ActivityLogMiner::mkHourPath(dlvid_type dlvId,const ::tm& tm)
{
  char buf[128];
  sprintf(makeDeliveryPath(buf,dlvId),
                      "activity_log/%04u.%02u.%02u/%02u",
                      tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                      tm.tm_hour);
  return path+buf;
}



namespace {
std::string::size_type skipField(const std::string& str,std::string::size_type pos)
{
  std::string::size_type rv=str.find(',',pos);
  if(rv==std::string::npos || rv+1==str.length())
  {
    throw InfosmeException(EXC_BADFORMAT,"Expected field separator wasn't found in line '%s' at pos %lu",str.c_str(),pos);
  }
  return rv+1;
}

}


bool ActivityLogMiner::getNext(int reqId, msgtime_type endTime,
                               ALMResult* result, bool& hasMore)
{
  hasMore = false;
  Request* req;
  {
    sync::MutexGuard mg(mtx);
    ReqMap::iterator it=reqMap.find(reqId);
    if(it==reqMap.end())
    {
      throw InfosmeException(EXC_EXPIRED,"request with id=%d expired or doesn't exists",reqId);
    }
    req=it->second;
    if(req->busy)
    {
      throw InfosmeException(EXC_LOGICERROR,"request with id=%d is processing at the moment",reqId);
    }
    req->busy=true;
    req->ref();
    // moving an expiration
    timeMap.erase(req->timeIt);
    req->timeIt = timeMap.insert(TimeMap::value_type(time(0)+requestTimeout,reqId));
  }
  try {
    bool rv=parseRecord(req,endTime,result,hasMore);
    sync::MutexGuard mg(mtx);
    req->unref();
    req->busy=false;
    return rv;
  } catch (std::exception& e)
  {
    sync::MutexGuard mg(mtx);
    req->unref();
    req->busy=false;
    throw;
  }
}

/*
int ActivityLogMiner::countRecords(dlvid_type dlvId,const ALMRequestFilter& filter)
{
  Request req;
  req.dlvId=dlvId;
  req.filter=filter;
  req.curDate=filter.startDate;
  req.offset=0;
  int rv=0;
  ALMResult res;
  while(parseRecord(&req,res))
  {
    rv++;
  }
  return rv;
}
 */


bool ActivityLogMiner::parseRecord(Request* req, msgtime_type endTime,
                                   ALMResult* result, bool& hasMore )
{
  using smsc::core::buffers::File;
  File& f=req->f;
  std::string filePath;
  bool nextFile=false;

  std::string line;
  std::string::size_type pos;
  int sec;

  bool& dayChecked=req->dayChecked;
  bool& hourChecked=req->hourChecked;
  int& day=req->day;
  int& hour=req->hour;

  for(;;)
  {
    if(!f.isOpened())
    {
      if(req->curDate>req->filter.endDate)
      {
        return false;
      }
      ::tm tm;
      msgTimeToYmd(req->curDate,&tm);
      if(!dayChecked || tm.tm_mday!=day)
      {
        if(!File::Exists(mkDatePath(req->dlvId,tm).c_str()))
        {
          req->curDate-=req->curDate%(24*60*60);
          req->curDate+=24*60*60;
          continue;
        }
        day=tm.tm_mday;
        dayChecked=true;
      }
      if(!hourChecked || tm.tm_hour!=hour)
      {
        if(!File::Exists(mkHourPath(req->dlvId,tm).c_str()))
        {
          req->curDate-=req->curDate%(60*60);
          req->curDate+=60*60;
          continue;
        }
        hour=tm.tm_hour;
        hourChecked=true;
      }

      filePath=mkFilePath(req->dlvId,tm);
      if(!nextFile)
      {
        smsc_log_debug(log,"probing file %s",filePath.c_str());
      }
      if(nextFile || !smsc::core::buffers::File::Exists(filePath.c_str()))
      {
        if(!nextFile)
        {
          smsc_log_debug(log,"file %s not found",filePath.c_str());
        }
        req->curDate-=req->curDate%60;
        req->curDate+=60;
        nextFile=false;
        continue;
      }
      smsc_log_debug(log,"reading %s",filePath.c_str());
      f.ROpen(filePath.c_str());
      if (!f.ReadLine(line))
      {
        f.Close();
        req->offset=0;
        nextFile=true;
        if (endTimeReached(endTime)) {
            hasMore = true;
            return false;
        }
        continue;
      }
      {
        int npos = 0;
        sscanf(line.c_str(),"#%u %n",&req->version,&npos);
        if (npos == 0)
        {
          f.Close();
          req->offset=0;
          nextFile=true;
          if (endTimeReached(endTime)) {
              hasMore = true;
              return false;
          }
          continue;
        }
      }
      f.Seek(req->offset);
    }

    if ( ++req->linesRead % 10000 == 0 ) {
        if (endTimeReached(endTime)) {
            hasMore = true;
            return false;
        }
    }

    if(!f.ReadLine(line))
    {
      f.Close();
      req->offset=0;
      nextFile=true;
      continue;
    }
    if(line.empty() || line[0]=='#')
    {
      continue;
    }

    //parse and push
    ALMResult* rec=result;
    if (rec) { rec->resultFields=req->filter.resultFields; }
    int n;
    pos=0;
    if(req->filter.resultFields&rfDate)
    {
      n=0;
      sscanf(line.c_str(),"%d,%n",&sec,&n);
      if(!n)
      {
        throw InfosmeException(EXC_BADFORMAT,"failed to parse seconds in line '%s'",line.c_str());
      }
      pos=n;
      if (rec) { rec->date=req->curDate+sec; }
    }else
    {
      pos=skipField(line,pos);
    }
    char st=line[pos];
    if(st=='B')
    {
      continue;
    }
    if((req->filter.resultFields&rfState) || !req->filter.stateFilter.empty())
    {
      MsgState state = MsgState(0);
      switch(st)
      {
        case 'N': state=MSGSTATE_INPUT;break;
        case 'P': state=MSGSTATE_PROCESS;break;
        case 'S': state=MSGSTATE_SENT;break;
        case 'R': state=MSGSTATE_RETRY;break;
        case 'D': state=MSGSTATE_DELIVERED;break;
        case 'F': state=MSGSTATE_FAILED;break;
        case 'E': state=MSGSTATE_EXPIRED;break;
        case 'K': state=MSGSTATE_KILLED;break;
      }
      if(!req->filter.stateFilter.empty() && req->filter.stateFilter.find(state)==req->filter.stateFilter.end())
      {
        continue;
      }
      if (rec) { rec->state = state; }
    }
    pos=skipField(line,pos);//skip state
    pos=skipField(line,pos);//skip regionId
    n=0;
    msgid_type msgId;
    sscanf(line.c_str()+pos,"%lld,%n",&msgId,&n);
    if(!n)
    {
      throw InfosmeException(EXC_BADFORMAT,"failed to parse msgid in line '%s'",line.c_str());
    }
    if (rec) {rec->id = msgId;}
    pos+=n;
    pos=skipField(line,pos);//skip retry
    pos=skipField(line,pos);//skip plan
    if((req->filter.resultFields&rfAbonent) || req->filter.abonentFilter.Count())
    {
      std::string::size_type np=line.find(',',pos);
      smsc::core::buffers::FixedLengthString<32> abonent, *abptr;
      abptr = rec ? &(rec->abonent) : &abonent;
      abptr->assign(line.c_str()+pos,np-pos);
      if(req->filter.abonentFilter.Count() && !req->filter.abonentFilter.Find(abptr->c_str()))
      {
        continue;
      }
      pos=np+1;
    }else
    {
      pos=skipField(line,pos);
    }
    pos=skipField(line,pos);//skip ttl
    if((req->filter.resultFields&rfErrorCode) || !req->filter.codeFilter.empty())
    {
      n=0;
      int code;
      sscanf(line.c_str()+pos,"%d,%n",&code,&n);
      if(!n)
      {
        throw InfosmeException(EXC_BADFORMAT,"failed to parse smpp error code in line '%s'",line.c_str());
      }
      if(!req->filter.codeFilter.empty() && req->filter.codeFilter.find(code)==req->filter.codeFilter.end())
      {
        continue;
      }
      if (rec) { rec->code = code; }
      pos+=n;
    }else
    {
      pos=skipField(line,pos);
    }
    if(req->filter.resultFields&rfUserData)
    {
      std::string::size_type np=line.find(',',pos);
      // smsc::core::buffers::FixedLengthString<Message::USERDATA_LENGTH> userData, *uptr;
      if (rec) {
          // std::string userData, *uptr;
          // uptr = rec ? &(rec->userData) : &userData;
          rec->userData.assign(line.c_str()+pos,np-pos);
      }
      pos=np+1;
    }else
    {
      pos=skipField(line,pos);
    }
    if(req->version > 1)
    {
      // FIXME: impl reading FLAGS according to version
      pos=skipField(line,pos);
    }
    if(req->filter.resultFields&rfText)
    {
      if(line[pos]=='"')
      {
        pos++;
      }else
      {
        throw InfosmeException(EXC_BADFORMAT,"expected message text in line '%s' (%d)",line.c_str(),pos);
      }
      std::string text, *tptr;
      tptr = rec ? &(rec->text) : &text;
      tptr->clear();
      tptr->reserve(line.length()-pos);
      for(;;pos++)
      {
        if(pos>=line.length())
        {
          throw InfosmeException(EXC_BADFORMAT,"unexpected end of line while parsing msg text '%s'",line.c_str());
        }
        if(line[pos]=='\\')
        {
          pos++;
          if(pos>=line.length())
          {
            throw InfosmeException(EXC_BADFORMAT,"unexpected end of line while parsing msg text '%s'",line.c_str());
          }
          if(line[pos]=='n')
          {
            *tptr += '\n';
          }else if(line[pos]=='\\')
          {
            *tptr += '\\';
          }else if(line[pos]=='"')
          {
            *tptr +='"';
          }else
          {
            throw InfosmeException(EXC_BADFORMAT,"unexpected escape symbol in msg text '%s' at pos %lu",line.c_str(),pos);
          }
        }else if(line[pos]=='"')
        {
          break;
        }else
        {
          *tptr+=line[pos];
        }
      }
    }
    break;
  }
  req->offset=f.Pos();
  hasMore = true;
  return true;
}

void ActivityLogMiner::pauseReq(int reqId)
{
  sync::MutexGuard mg(mtx);
  ReqMap::iterator it=reqMap.find(reqId);
  if(it==reqMap.end())
  {
    throw InfosmeException(EXC_EXPIRED,"request with id=%d expired or doesn't exists",reqId);
  }
  it->second->f.Close();
}



}
}
}
