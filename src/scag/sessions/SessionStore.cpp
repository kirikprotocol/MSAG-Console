#include "SessionStore.h"

namespace scag{
namespace sessions{

void SessionStore::init(const std::string& dir,SessionLoadCallback cb,void* dataPtr)
{
  log=smsc::logger::Logger::getInstance("sesstore");

  std::string idxFile="sessoionstore.idx";
  std::string binFile="sessoionstore.bin";

  if( dir.length()>0 && ( dir[dir.length()-1]=='\\' || dir[dir.length()-1]=='/') )
  {
    idxFile=dir+idxFile;
    binFile=dir+binFile;
  }else
  {
    idxFile=dir+'/'+idxFile;
    binFile=dir+'/'+binFile;
  }

  if(!buf::File::Exists(idxFile.c_str()))
  {
    dhash.Create(idxFile.c_str(),100000,false);
  }else
  {
    dhash.Open(idxFile.c_str());
  }
  if(!buf::File::Exists(binFile.c_str()))
  {
    pfile.Create(binFile.c_str(),256,25000);
  }else
  {
    pfile.Open(binFile.c_str());
  }

  File::offset_type idx=0,ridx;
  std::vector<unsigned char> data;
  while((idx=pfile.Read(idx,data,&ridx))!=0)
  {
    Session s=Session(CSessionKey());
    SessionBuffer sb(data.size());
    sb.Append((char*)&data[0],data.size());
    s.Deserialize(sb);
    cb(dataPtr,&s);
  }
}

SessionPtr SessionStore::getSession(const CSessionKey& sessionKey)
{
  sync::MutexGuard mg(mtx);
  OffsetValue off;
  if(!dhash.LookUp(sessionKey,off))
  {
    return SessionPtr(0);
  }
  std::vector<unsigned char> data;
  pfile.Read(off.value,data);
  SessionPtr s(new Session(sessionKey));
  SessionBuffer sb(data.size());
  sb.Append((char*)&data[0],data.size());
  s->Deserialize(sb);
  return s;
}

SessionPtr SessionStore::newSession(const CSessionKey& sessionKey)
{
  SessionBuffer sb;
  SessionPtr s(new Session(sessionKey));
  s->Serialize(sb);
  sync::MutexGuard mg(mtx);
  OffsetValue val(pfile.Append(sb.get(),sb.GetPos()));
  smsc_log_debug(log,"newSession:%s:%08llx",DiskSessionKey(sessionKey).toString().c_str(),val.value);
  dhash.Insert(sessionKey,val);
  return s;
}

void SessionStore::deleteSession(const CSessionKey& sessionKey)
{
  sync::MutexGuard mg(mtx);
  OffsetValue off;
  if(!dhash.LookUp(sessionKey,off))
  {
    smsc_log_info(log,"Attempt to delete session that doesn't exists:%s",DiskSessionKey(sessionKey).toString().c_str());
    return;
  }
  smsc_log_debug(log,"delSession:%s:%08llx",DiskSessionKey(sessionKey).toString().c_str(),off.value);
  pfile.Delete(off.value);
  dhash.Delete(sessionKey);
}

void SessionStore::updateSession(SessionPtr session)
{
  sync::MutexGuard mg(mtx);
  OffsetValue off;
  if(!dhash.LookUp(session->getSessionKey(),off))
  {
    throw smsc::util::Exception("SessionStore::updateSession - Session with key %s not found",DiskSessionKey(session->getSessionKey()).toString().c_str());
  }
  smsc_log_debug(log,"updateSession:%s:%08llx",DiskSessionKey(session->getSessionKey()).toString().c_str(),off.value);
  SessionBuffer sb;
  session->Serialize(sb);
  pfile.Update(off.value,sb.get(),sb.GetPos());
}

}
}
