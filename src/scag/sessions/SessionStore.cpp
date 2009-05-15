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

  if(buf::File::Exists(idxFile.c_str()))
  {
    buf::File::Unlink(idxFile.c_str());
  }
  dhash.Create(idxFile.c_str(),100000,false);

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
    CSessionKey key;
    Session s(key);
    SessionBuffer sb(data.size());
    sb.Append((char*)&data[0],data.size());
    s.Deserialize(sb);
    s.getOffsetRef()=ridx;
    cb(dataPtr,&s);
  }
}

void SessionStore::storeSessionIndex(Session* session)
{
  sync::MutexGuard mg(mtx);
  OffsetValue off(session->getOffsetRef());
  const CSessionKey& key=session->getSessionKey();
  dhash.Insert(key,off);
}


void SessionStore::deleteSessionByIndex(Session* session)
{
  sync::MutexGuard mg(mtx);
  OffsetValue off=session->getOffsetRef();
  if(off.value)
  {
    pfile.Delete(off.value);
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

  smsc_log_debug(log, "perform lookup");
  if(!dhash.LookUp(sessionKey,off))
  {
    smsc_log_info(log,"Attempt to delete session that doesn't exists:%s",DiskSessionKey(sessionKey).toString().c_str());
    return;
  }
  smsc_log_debug(log,"delSession:%s:%08llx",DiskSessionKey(sessionKey).toString().c_str(),off.value);

  pfile.Delete(off.value);
  smsc_log_debug(log, "pfile.Delete() - passed");

  dhash.Delete(sessionKey);
  smsc_log_debug(log, "dhash.Delete() - passed");

}

bool SessionStore::updateSession(Session* session)
{
  sync::MutexGuard mg(mtx);
  OffsetValue off;
  if(!dhash.LookUp(session->getSessionKey(),off))
  {
    smsc_log_error(log, "SessionStore::updateSession - Session with key %s not found",DiskSessionKey(session->getSessionKey()).toString().c_str());
    return false;
  }

  smsc_log_debug(log,"updateSession:%s:%08llx",DiskSessionKey(session->getSessionKey()).toString().c_str(),off.value);
  SessionBuffer sb;
  session->Serialize(sb);
  pfile.Update(off.value,sb.get(),sb.GetPos());
  return true;
}


void CachedSessionStore::init(const std::string& dir,SessionLoadCallback cb,void* dataPtr, uint32_t mcs)
{
    maxCacheSize = mcs;
    cache = new SessionPtr[mcs];
    store.init(dir, cb, dataPtr);
}

SessionPtr CachedSessionStore::getSession(const CSessionKey& sessionKey)
{
  sync::MutexGuard mg(mtx);

  uint32_t i = getIdx(sessionKey);
//  smsc_log_debug(logger, "getSession: %p %s:%d %s:%d, hit: %d", cache[i].Get(), cache[i].Get() ? cache[i]->getSessionKey().abonentAddr.toString().c_str() : "", cache[i].Get() ? cache[i]->getSessionKey().USR : 0, sessionKey.abonentAddr.toString().c_str(), sessionKey.USR, cache[i].Get() && cache[i]->getSessionKey() == sessionKey);
  if(cache[i].Get() && cache[i]->getSessionKey() == sessionKey)
    return cache[i];

  SessionPtr p = store.getSession(sessionKey);
  if(p.Get())
    cache[i] = p;

  return p;
}

SessionPtr CachedSessionStore::newSession(const CSessionKey& sessionKey)
{
  sync::MutexGuard mg(mtx);
  SessionPtr p = store.newSession(sessionKey);
  cache[getIdx(sessionKey)] = p;
  return p;
}

void CachedSessionStore::deleteSession(const CSessionKey& sessionKey)
{
  sync::MutexGuard mg(mtx);
  uint32_t i = getIdx(sessionKey);
  if(cache[i].Get() && cache[i]->getSessionKey() == sessionKey)
    cache[i] = NULL;
  store.deleteSession(sessionKey);
}

bool CachedSessionStore::updateSession(Session* session)
{
  sync::MutexGuard mg(mtx);
  return store.updateSession(session);
}

}
}
