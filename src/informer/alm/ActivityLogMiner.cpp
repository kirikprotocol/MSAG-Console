#include "ActivityLogMiner.hpp"
#include "informer/io/InfosmeException.h"
#include "core/buffers/File.hpp"
#include "informer/data/CommonSettings.h"

namespace eyeline{
namespace informer{
namespace alm{

using smsc::core::buffers::File;

smsc::logger::Logger* ActivityLogMiner::log = 0;
std::string ActivityLogMiner::path;

ActivityLogMiner::ActivityLogMiner() :
reqIdSeq(0)
{
  requestTimeout=60*60;
  if (!log) {
      log = smsc::logger::Logger::getInstance("alm");
  }
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
    bool rv = req->parseRecord(endTime,result,hasMore);
    // req->show("after parseRec");
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


bool ActivityLogMiner::Request::openNextFile( msgtime_type endTime, bool& hasMore )
{
    bool res = false;
    show("openFile");
    while ( !f.isOpened() ) {

        if ( curDate > filter.endDate ) {
            break;
        }

        ::tm tm;
        msgTimeToYmd( curDate, &tm );

        if ( tm.tm_mday != day || (zipVersion && offset && hour==-1) ) {

            day = tm.tm_mday;
            hour = -1;

            const std::string daypath = mkDatePath(dlvId,tm);
            if ( File::Exists((daypath+".log").c_str()) ) {
                // zipped
                smsc_log_debug(log,"zipped day log %s found",(daypath+".log").c_str());
                f.ROpen((daypath+".log").c_str());
                if ( offset && zipVersion ) {
                    res = true;
                    break;
                }
                if ( !readZipVersion() ||
                     !scanZipToDate() ||
                     !readChunkVersion() ) {
                    advance(true);
                    continue;
                }
                res = true;
                break;
            }
            
            if ( !File::Exists(daypath.c_str()) ) {
                // no day dir
                advance(true);
                offset = 0;
                continue;
            }
        }

        if ( tm.tm_hour != hour || (zipVersion && offset) ) {

            hour = tm.tm_hour;

            const std::string hourpath = mkHourPath(dlvId,tm);
            if ( File::Exists((hourpath+".log").c_str()) ) {
                // zipped
                smsc_log_debug(log,"zipped hour log %s found",(hourpath+".log").c_str());
                f.ROpen((hourpath+".log").c_str());
                if ( offset && zipVersion ) {
                    res = true;
                    break;
                }
                if ( !readZipVersion() ||
                     !scanZipToDate() ||
                     !readChunkVersion() ) {
                    advance(true);
                    continue;
                }
                res = true;
                break;
            }
            
            if ( !File::Exists(hourpath.c_str()) ) {
                // no hour dir
                advance(true);
                offset = 0;
                continue;
            }
        }

        const std::string filePath = mkFilePath(dlvId,tm);
        // smsc_log_debug(log,"probing file %s",filePath.c_str());
        if ( !File::Exists(filePath.c_str()) ) {
            // smsc_log_debug(log,"file %s is not found",filePath.c_str());
            advance(false);
            offset = 0;
            continue;
        }

        // smsc_log_debug(log,"reading %s",filePath.c_str());
        f.ROpen(filePath.c_str());
        // opened, round up curDate
        curDate -= curDate % 60;
        if ( !offset && !readChunkVersion() ) {
            advance(false);
            if (endTimeReached(endTime)) {
                hasMore = true;
                break;
            }
            continue;
        }
        res = true;
        break;
    }
    show("after openFile");
    if (res) f.Seek(offset);
    return res;
}


bool ActivityLogMiner::Request::readZipVersion()
{
    version = 0;
    nextzipchunk = 0;
    std::string line;
    offset = 0;
    if (f.ReadLine(line)) {
        int pos = 0;
        unsigned v;
        sscanf(line.c_str(),"#%u ZIPPED%n",&v,&pos);
        if (pos!=0 && v) {
            zipVersion = v;
            return true;
        }
        smsc_log_warn(log,"ziplog %s version line is broken near %lu",
                      f.getFileName().c_str(),
                      static_cast<unsigned long>(f.Pos()));
    } else {
        smsc_log_warn(log,
                      "ziplog %s version line is missed near %lu",
                      f.getFileName().c_str(),
                      static_cast<unsigned long>(f.Pos()));
    }
    f.Close();
    return false;
}


bool ActivityLogMiner::Request::scanZipToDate()
{
    offset = 0;
    if (!zipVersion) {
        smsc_log_warn(log,"zipVersion is not set");
        f.Close();
        return false;
    }
    do {
        msgtime_type rt = readZipChunkHead();
        if (!rt) break;
        if ( rt <= curDate ) {
            // check that next chunk is too big
            if ( rt+60 > curDate ) {
                // we are here!
                // round up the curDate
                curDate = rt;
                return true;
            }
            // we have not reached curDate yet
            // skip the chunk completely
            f.Seek( nextzipchunk );
            continue;
        }
        // here rt > curDate
        curDate = rt;
        if (curDate > filter.endDate) {
            // too late
            break;
        }
        return true;
    } while (false);
    f.Close();
    return false;
}
    

msgtime_type ActivityLogMiner::Request::readZipChunkHead()
{
    version = 0;
    std::string line;
    if (f.ReadLine(line)) {
        int pos = 0;
        unsigned year, month, mday, rhour, rminute;
        unsigned long chunksize;
        sscanf(line.c_str(),"# %04u %02u %02u %02u %02u %lu%n",
               &year, &month, &mday, &rhour, &rminute, &chunksize,
               &pos );
        if (pos!=0) {
            nextzipchunk = f.Pos() + chunksize;
            const msgtime_type rt = ymdToMsgTime( ((((ulonglong(year)*100+month)*100+mday)*100+rhour)*100+rminute)*100 );
            if (log->isDebugEnabled()) {
                char buf[100];
                sprintf(buf,"readZCH(%llu,%lu)",msgTimeToYmd(rt),chunksize);
                show(buf);
            }
            return rt;
        }
        smsc_log_warn(log,"ziplog %s chunkline is broken near %lu",
                      f.getFileName().c_str(),
                      static_cast<unsigned long>(f.Pos()));
    } else {
        smsc_log_warn(log,"ziplog %s has no chunkline near %lu",
                      f.getFileName().c_str(),
                      static_cast<unsigned long>(f.Pos()));
    }
    f.Close();
    offset = 0;
    return 0;
}


bool ActivityLogMiner::Request::readChunkVersion()
{
    std::string line;
    if ( f.ReadLine(line) ) {
        int pos = 0;
        unsigned v;
        sscanf(line.c_str(),"#%u %n",&v,&pos);
        if (pos != 0) {
            version = v;
            offset = f.Pos();
            return true;
        }
        smsc_log_warn(log,"file %s version line is broken near %lu",
                      f.getFileName().c_str(),
                      static_cast<unsigned long>(f.Pos()));
    } else {
        smsc_log_warn(log,
                      "file %s version line is missed near %lu",
                      f.getFileName().c_str(),
                      static_cast<unsigned long>(f.Pos()));
    }
    f.Close();
    offset = 0;
    return false;
}


void ActivityLogMiner::Request::show( const char* where )
{
    if (log->isDebugEnabled()) {
        const char* fn = "";
        if ( f.isOpened() ) {
            fn = f.getFileName().c_str();
            const char* x = strrchr(fn,'/');
            if (x) fn = x+1;
        }
        smsc_log_debug(log,"%s D=%u cur=%llu zv=%u v=%u d=%d h=%d ln=%u off=%llu %s",
                       where,
                       unsigned(dlvId),
                       msgTimeToYmd(curDate), zipVersion, version, day, hour,
                       linesRead,
                       static_cast<unsigned long long>(offset),
                       fn);
    }
}


bool ActivityLogMiner::Request::parseRecord( msgtime_type endTime,
                                             ALMResult* result,
                                             bool& hasMore )
{
    std::string line;
    int sec;

    // show("parseRecord");

    for(;;) {

        if (!f.isOpened()) {
            if ( !openNextFile(endTime,hasMore) ) {
                return false;
            }
        }
            
        if ( ++linesRead % 10000 == 0 ) {
            if (endTimeReached(endTime)) {
                hasMore = true;
                return false;
            }
        }

        // check if we have zipped file
        if ( zipVersion ) {
            const size_t fpos = f.Pos();
            if ( fpos >= f.Size() ) {
                // eof reached
                f.Close();
                offset = 0;
                advance(true);
                continue;
            }
            if ( fpos >= nextzipchunk ) {
                msgtime_type rt = readZipChunkHead();
                if (!rt) {
                    f.Close();
                    advance(true);
                    continue;
                }
                if (rt < curDate) {
                    smsc_log_warn(log,"logic failure: curDate=%llu rt=%llu from readZCH in %s",
                                  msgTimeToYmd(curDate),
                                  msgTimeToYmd(rt),
                                  f.getFileName().c_str());
                    f.Close();
                    offset = 0;
                    return false;
                    // advance(true);
                    // continue;
                }
                if (rt >= curDate) {
                    curDate = rt;
                    if (curDate > filter.endDate) {
                        f.Close();
                        offset = 0;
                        return false;
                    }
                }
                readChunkVersion();
            }
        }


        if(!f.ReadLine(line))
        {
            f.Close();
            offset = 0;
            advance(zipVersion != 0);
            continue;
        }

        if(line.empty() || line[0]=='#')
        {
            continue;
        }

        //parse and push
        ALMResult* rec=result;
        if (rec) { rec->resultFields=filter.resultFields; }
        int n;
        std::string::size_type pos = 0;
        if(filter.resultFields&rfDate)
        {
            n=0;
            sscanf(line.c_str(),"%d,%n",&sec,&n);
            if(!n)
            {
                throw InfosmeException(EXC_BADFORMAT,"failed to parse seconds in line '%s'",line.c_str());
            }
            pos=n;
            if (rec) { rec->date=curDate+sec; }
        }else
        {
            pos=skipField(line,pos);
        }
        char st=line[pos];
        if(st=='B')
        {
            continue;
        }
        if((filter.resultFields&rfState) || !filter.stateFilter.empty())
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
            if(!filter.stateFilter.empty() && filter.stateFilter.find(state)==filter.stateFilter.end())
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
        if((filter.resultFields&rfAbonent) || filter.abonentFilter.Count())
        {
            std::string::size_type np=line.find(',',pos);
            smsc::core::buffers::FixedLengthString<32> abonent, *abptr;
            abptr = rec ? &(rec->abonent) : &abonent;
            abptr->assign(line.c_str()+pos,np-pos);
            if(filter.abonentFilter.Count() && !filter.abonentFilter.Find(abptr->c_str()))
            {
                continue;
            }
            pos=np+1;
        }else
        {
            pos=skipField(line,pos);
        }
        pos=skipField(line,pos);//skip ttl
        if((filter.resultFields&rfErrorCode) || !filter.codeFilter.empty())
        {
            n=0;
            int code;
            sscanf(line.c_str()+pos,"%d,%n",&code,&n);
            if(!n)
            {
                throw InfosmeException(EXC_BADFORMAT,"failed to parse smpp error code in line '%s'",line.c_str());
            }
            if(!filter.codeFilter.empty() && filter.codeFilter.find(code)==filter.codeFilter.end())
            {
                continue;
            }
            if (rec) { rec->code = code; }
            pos+=n;
        }else
        {
            pos=skipField(line,pos);
        }
        if(filter.resultFields&rfUserData)
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
        if(version > 1)
        {
            // FIXME: impl reading FLAGS according to version
            pos=skipField(line,pos);
        }
        if(filter.resultFields&rfText)
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
    offset = f.Pos();
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
  it->second->closeFile();
  it->second->show("pauseReq");
}



}
}
}
