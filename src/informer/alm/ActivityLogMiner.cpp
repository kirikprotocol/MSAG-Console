#include "ActivityLogMiner.hpp"
#include "informer/io/InfosmeException.h"
#include "core/buffers/File.hpp"

namespace eyeline{
namespace informer{
namespace alm{

ActivityLogMiner::ActivityLogMiner():reqIdSeq(0)
{
  requestTimeout=60*60;
}

void ActivityLogMiner::init(const std::string& argPath,time_t argRequestTimeout)
{
  path=argPath;
  requestTimeout=argRequestTimeout;
}


int ActivityLogMiner::createRequest(dlvid_type dlvId,const ALMRequestFilter& filter)
{
  Request* req=new Request;
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
      delete it->second;
      reqMap.erase(it);
    }
    timeMap.erase(tit);
  }
  int rv=reqIdSeq++;
  req->timeIt=timeMap.insert(TimeMap::value_type(now+requestTimeout,rv));
  return rv;
}

std::string ActivityLogMiner::mkFilePath(dlvid_type dlvId,msgtime_type date)
{
  char buf[128];
  ::tm tm;
  msgTimeToYmd(date,&tm);
  sprintf(makeDeliveryPath(buf,dlvId),
                      "activity_log/%04u.%02u.%02u/%02u/%02u.log",
                      tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                      tm.tm_hour, tm.tm_min );
  return path+buf;
}

namespace {
std::string::size_type skipField(const std::string& str,std::string::size_type pos)
{
  std::string::size_type rv=str.find(',');
  if(rv==std::string::npos || rv+1==str.length())
  {
    throw InfosmeException(EXC_BADFORMAT,"Expected field separator wasn't found in line '%s' at pos %lu",str.c_str(),pos);
  }
  return rv+1;
}

}


bool ActivityLogMiner::getNext(int reqId,std::vector<ALMResult>& result,int count)
{
  sync::MutexGuard mg(mtx);
  ReqMap::iterator it=reqMap.find(reqId);
  if(it==reqMap.end())
  {
    throw InfosmeException(EXC_EXPIRED,"request with id=%d expired or doesn't exists",reqId);
  }
  result.reserve(count);
  return parseFiles(it->second,&result,count,false)==count;
}

int ActivityLogMiner::countRecords(dlvid_type dlvId,const ALMRequestFilter& filter)
{
  Request req;
  req.dlvId=dlvId;
  req.filter=filter;
  req.curDate=filter.startDate;
  req.offset=0;
  return parseFiles(&req,0,0,true);
}


int ActivityLogMiner::parseFiles(Request* req,std::vector<ALMResult>* result,int count,bool countOnly)
{
  smsc::core::buffers::File f;
  std::string filePath;
  bool nextFile=false;

  std::string line;
  std::string::size_type pos;
  int sec;

  int rv=0;

  while(countOnly || rv<count)
  {
    if(!f.isOpened())
    {
      filePath=mkFilePath(req->dlvId,req->curDate);
      if(nextFile || !smsc::core::buffers::File::Exists(filePath.c_str()))
      {
        req->curDate+=60;
        if(req->curDate>req->filter.endDate)
        {
          return rv;
        }
        continue;
      }
      f.ROpen(filePath.c_str());
      f.Seek(req->offset);
    }
    if(!f.ReadLine(line))
    {
      f.Close();
      nextFile=true;
      continue;
    }
    if(line.empty() || line[0]=='#')
    {
      continue;
    }

    //parse and push
    ALMResult rec;
    int n;
    pos=0;
    if(req->filter.resultFields&rfDate)
    {
      if(!sscanf(line.c_str(),"%d,%n",&sec,&n)!=1)
      {
        throw InfosmeException(EXC_BADFORMAT,"failed to parse seconds in line '%s'",line.c_str());
      }
      pos=n;
      rec.date=req->curDate+sec;
    }else
    {
      pos=skipField(line,pos);
    }
    if((req->filter.resultFields&rfState) || !req->filter.stateFilter.empty())
    {
      switch(line[pos])
      {
        case 'N': rec.state=MSGSTATE_INPUT;break;
        case 'P': rec.state=MSGSTATE_PROCESS;break;
        case 'R': rec.state=MSGSTATE_RETRY;break;
        case 'D': rec.state=MSGSTATE_DELIVERED;break;
        case 'F': rec.state=MSGSTATE_FAILED;break;
        case 'E': rec.state=MSGSTATE_EXPIRED;break;
      }
      if(!req->filter.stateFilter.empty() && req->filter.stateFilter.find(rec.state)==req->filter.stateFilter.end())
      {
        continue;
      }
    }
    pos=skipField(line,pos);//skip state
    pos=skipField(line,pos);//skip regionId
    if(sscanf(line.c_str(),"%lld,%n",&rec.id,&n)!=1)
    {
      throw InfosmeException(EXC_BADFORMAT,"failed to parse msgid in line '%s'",line.c_str());
    }
    pos=n;
    pos=skipField(line,pos);//skip retry
    pos=skipField(line,pos);//skip plan
    if((req->filter.resultFields&rfAbonent) || !req->filter.abonentFilter.empty())
    {
      std::string::size_type np=line.find(',');
      rec.abonent.assign(line.c_str()+pos,pos-np);
      if(!req->filter.abonentFilter.empty() && req->filter.abonentFilter.find(rec.abonent.c_str())==req->filter.abonentFilter.end())
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
      if(sscanf(line.c_str(),"%d,%n",&rec.code,&n)!=1)
      {
        throw InfosmeException(EXC_BADFORMAT,"failed to parse smpp error code in line '%s'",line.c_str());
      }
      if(!req->filter.codeFilter.empty() && req->filter.codeFilter.find(rec.code)==req->filter.codeFilter.end())
      {
        continue;
      }
      pos=n;
    }else
    {
      pos=skipField(line,pos);
    }
    if(req->filter.resultFields&rfUserData)
    {
      std::string::size_type np=line.find(',');
      rec.userData.assign(line.c_str()+pos,pos-np);
      pos=np+1;
    }else
    {
      pos=skipField(line,pos);
    }
    if(req->filter.resultFields&rfText)
    {
      if(line[pos]=='"')
      {
        pos++;
      }else
      {
        throw InfosmeException(EXC_BADFORMAT,"expected message text in line '%s'",line.c_str());
      }
      rec.text.reserve(line.length()-pos);
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
            rec.text+='\n';
          }else if(line[pos]=='\\')
          {
            rec.text+='\\';
          }else if(line[pos]=='"')
          {
            rec.text+='"';
          }else
          {
            throw InfosmeException(EXC_BADFORMAT,"unexpected escape symbol in msg text '%s' at pos %lu",line.c_str(),pos);
          }
        }else if(line[pos]=='"')
        {
          break;
        }else
        {
          rec.text+=line[pos];
        }
      }
    }
    if(result)
    {
      result->push_back(rec);
    }
    rv++;
  }
  req->offset=f.Pos();
  return rv;
}


}
}
}
