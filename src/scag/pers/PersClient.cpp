/* $Id$ */

#include <poll.h>
#include "logger/Logger.h"
#include <core/threads/Thread.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <scag/util/singleton/Singleton.h>
#include "scag/config/ConfigManager.h"
#include "scag/config/ConfigListener.h"
#include "scag/lcm/LongCallManager.h"

#include "PersClient.h"

namespace scag { namespace pers { namespace client {

using namespace scag::util::singleton;
using smsc::util::Exception;
using smsc::logger::Logger;
using smsc::core::threads::Thread;
using namespace scag::lcm;
using namespace scag::pers;

bool  PersClient::inited = false;
Mutex PersClient::initLock;

class PersClientImpl: public PersClient, public scag::config::ConfigListener, public Thread {
//    friend class PersClient;
public:
    PersClientImpl(): connected(false), headContext(NULL), ConfigListener(scag::config::PERSCLIENT_CFG) {};
    ~PersClientImpl() { Stop(); if(connected) sock.Close(); };

    void SetProperty(ProfileType pt, const PersKey& key, Property& prop);
    void GetProperty(ProfileType pt, const PersKey& key, const char *property_name, Property& prop);
    bool DelProperty(ProfileType pt, const PersKey& key, const char *property_name);
    int IncProperty(ProfileType pt, const PersKey& key, Property& prop);
    int IncModProperty(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod);
    
    void SetPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb);
    void GetPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb);
    void DelPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb);
    void IncPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb);
    void IncModPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod, SerialBuffer& bsb);

    void SetPropertyPrepare(Property& prop, SerialBuffer& bsb);
    void GetPropertyPrepare(const char *property_name, SerialBuffer& bsb);
    void DelPropertyPrepare(const char *property_name, SerialBuffer& bsb);
    void IncPropertyPrepare(Property& prop, SerialBuffer& bsb);
    void IncModPropertyPrepare(Property& prop, uint32_t mod, SerialBuffer& bsb);
    
    void SetPropertyResult(SerialBuffer& bsb);
    void GetPropertyResult(Property& prop, SerialBuffer& bsb);
    bool DelPropertyResult(SerialBuffer& bsb);
    int IncPropertyResult(SerialBuffer& bsb);
    int IncModPropertyResult(SerialBuffer& bsb);
	
	void PrepareBatch(SerialBuffer& bsb, bool transactMode = false);
    void PrepareMTBatch(SerialBuffer& bsb, ProfileType pt, const PersKey& key, uint16_t cnt, bool transactMode = false);
	void FinishPrepareBatch(uint32_t cnt, SerialBuffer& bsb);
	void RunBatch(SerialBuffer& bsb);

    bool call(LongCallContext* context);

    void init_internal(const char *_host, int _port, int timeout, int _pingTimeout);
    
    virtual int Execute();
    void Stop();
    void Start() {isStopping = false; Thread::Start();};
	
protected:
    void setBatchCount(uint32_t cnt, SerialBuffer& bsb);    
    void configChanged();
    void init();
    void reinit(const char *_host, int _port, int _timeout, int _pingTimeout);
    void setPacketSize(SerialBuffer& bsb);
	
	void emptyPacket(SerialBuffer& bsb) { bsb.Empty(); bsb.SetPos(4); }
	
    void FillHead(PersCmd pc, ProfileType pt, const PersKey& key, SerialBuffer& bsb);
    void AppendString(const char *str);
    void SendPacket(SerialBuffer& bsb);
    void ReadPacket(SerialBuffer& bsb);
    void WriteAllTO(const char* buf, uint32_t sz);
    void ReadAllTO(char* buf, uint32_t sz);
    uint32_t getPacketSize(SerialBuffer& bsb);
    PersServerResponseType GetServerResponse(SerialBuffer& bsb);
    void ParseProperty(Property& prop, SerialBuffer& bsb);

    LongCallContext* getContext();
    void ping();
    void ExecutePersCall(LongCallContext* ctx);

    void CheckServerResponse(SerialBuffer& bsb);


    std::string host;
    int port;
    int timeout, pingTimeout;
    time_t actTS;
    Socket sock;
    bool connected, isStopping;
    Logger * log;
    Mutex mtx;
    EventMonitor callMonitor;
    SerialBuffer sb;
    LongCallContext* headContext, *tailContext;
};

inline unsigned GetLongevity(PersClient*) { return 5; }
typedef SingletonHolder<PersClientImpl> SinglePC;

PersClient& PersClient::Instance()
{
    if (!PersClient::inited)
    {
        MutexGuard guard(PersClient::initLock);
        if (!PersClient::inited) 
            throw std::runtime_error("PersClient not inited!");
    }
    return SinglePC::Instance();
}

void PersClient::Init(const char *_host, int _port, int _timeout, int _pingTimeout) //throw(PersClientException)
{
    if (!PersClient::inited)
    {
        MutexGuard guard(PersClient::initLock);
        if(!inited) {
            PersClientImpl& pc = SinglePC::Instance();
            pc.init_internal(_host, _port, _timeout, _pingTimeout);
            PersClient::inited = true;
        }
    }
}

void PersClient::Init(const scag::config::PersClientConfig& cfg)// throw(PersClientException);    
{
    if (!PersClient::inited)
    {
        MutexGuard guard(PersClient::initLock);
        if(!inited) {
            PersClientImpl& pc = SinglePC::Instance();
            pc.init_internal(cfg.host.c_str(), cfg.port, cfg.timeout, cfg.pingTimeout);
            PersClient::inited = true;
        }
    }
} 

void PersClientImpl::init_internal(const char *_host, int _port, int _timeout, int _pingTimeout) //throw(PersClientException)
{
    log = Logger::getInstance("persclient");
    smsc_log_info(log, "PersClient init host=%s:%d timeout=%d, pingtimeout=%d", _host, _port, _timeout, _pingTimeout);
    connected = false;
    host = _host;
    port = _port;
    timeout = _timeout;
    pingTimeout = _pingTimeout;
    actTS = time(NULL);
    try{
        init();
    }
    catch(PersClientException& e)
    {
        smsc_log_error(log, "Error during initialization. %s", e.what());
    }
    Start();
}

void PersClientImpl::configChanged()
{
    scag::config::PersClientConfig& cfg = scag::config::ConfigManager::Instance().getPersClientConfig();
    
    reinit(cfg.host.c_str(), cfg.port, cfg.timeout, cfg.pingTimeout);
}

void PersClientImpl::reinit(const char *_host, int _port, int _timeout, int _pingTimeout) //throw(PersClientException)
{
    MutexGuard mt(mtx);

    smsc_log_info(log, "PersClient reinit host=%s:%d timeout=%d, pingtimeout=%d", _host, _port, _timeout, _pingTimeout);
    
    if(connected) sock.Close();
    Stop();
    connected = false;
    host = _host;
    port = _port;
    timeout = _timeout;
    pingTimeout = _pingTimeout;
    actTS = time(NULL);
    try{
        init();
    }
    catch(PersClientException& e)
    {
        smsc_log_error(log, "Error during reinitialization. %s", e.what());
    }
    Start();
}

void PersClientImpl::SetPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb)
{
    FillHead(PC_SET, pt, key, bsb);
    prop.Serialize(bsb);	
}

void PersClientImpl::SetPropertyResult(SerialBuffer& bsb)
{
    //if(GetServerResponse(bsb) != RESPONSE_OK)
      //  throw PersClientException(SERVER_ERROR);
  CheckServerResponse(bsb);
}

void PersClientImpl::SetProperty(ProfileType pt, const PersKey& key, Property& prop)
{
    MutexGuard mt(mtx);

	emptyPacket(sb);
	SetPropertyPrepare(pt, key, prop, sb);
    SendPacket(sb);
   	ReadPacket(sb);
	SetPropertyResult(sb);
}

void PersClientImpl::GetPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb)
{
    FillHead(PC_GET, pt, key, bsb);
    bsb.WriteString(property_name);
}

void PersClientImpl::GetPropertyResult(Property& prop, SerialBuffer& bsb)
{
    //ParseProperty(prop, bsb);
  CheckServerResponse(bsb);
  prop.Deserialize(bsb);
}

void PersClientImpl::GetProperty(ProfileType pt, const PersKey& key, const char *property_name, Property& prop)
{
    MutexGuard mt(mtx);

    emptyPacket(sb);
	GetPropertyPrepare(pt, key, property_name, sb);
    SendPacket(sb);
    ReadPacket(sb);
	GetPropertyResult(prop, sb);
    actTS = time(NULL);
}

void PersClientImpl::DelPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb)
{
    FillHead(PC_DEL, pt, key, bsb);
    bsb.WriteString(property_name);
}

bool PersClientImpl::DelPropertyResult(SerialBuffer& bsb)
{
/*
	int resp;
	
	try{
	    resp = bsb.ReadInt8();
	}
	catch(SerialBufferOutOfBounds& e)
	{
		throw PersClientException(BAD_RESPONSE);
	}

    if(resp != RESPONSE_OK && resp != RESPONSE_PROPERTY_NOT_FOUND)
        throw PersClientException(SERVER_ERROR);
		
	return resp != RESPONSE_PROPERTY_NOT_FOUND;
    */
  CheckServerResponse(bsb);
  return  true;
}

bool PersClientImpl::DelProperty(ProfileType pt, const PersKey& key, const char *property_name)
{
    MutexGuard mt(mtx);

	emptyPacket(sb);
	DelPropertyPrepare(pt, key, property_name, sb);
    SendPacket(sb);
    ReadPacket(sb);
    bool b = DelPropertyResult(sb);
    actTS = time(NULL);
	return b;
}

void PersClientImpl::IncPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb)
{
    if(prop.getType() != INT && prop.getType() != DATE)
        throw PersClientException(INVALID_PROPERTY_TYPE);

    //FillHead(PC_INC, pt, key, bsb);
    FillHead(PC_INC_RESULT, pt, key, bsb);
    prop.Serialize(bsb);
}

int PersClientImpl::IncPropertyResult(SerialBuffer& bsb)
{
  return IncModPropertyResult(bsb);
}

int PersClientImpl::IncProperty(ProfileType pt, const PersKey& key, Property& prop)
{
    MutexGuard mt(mtx);

    emptyPacket(sb);
    IncPropertyPrepare(pt, key, prop, sb);
    SendPacket(sb);
    ReadPacket(sb);
    int i = IncPropertyResult(sb);
		
    actTS = time(NULL);
    return i;
}

void PersClientImpl::IncModPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod, SerialBuffer& bsb)
{
    if(prop.getType() != INT && prop.getType() != DATE)
        throw PersClientException(INVALID_PROPERTY_TYPE);

    FillHead(PC_INC_MOD, pt, key, bsb);
    bsb.WriteInt32(mod);
    prop.Serialize(bsb);
}

void PersClientImpl::SetPropertyPrepare(Property& prop, SerialBuffer& bsb) {
  bsb.WriteInt8(PC_SET);
  prop.Serialize(bsb);
}

void PersClientImpl::GetPropertyPrepare(const char *property_name, SerialBuffer& bsb) {
  bsb.WriteInt8(PC_GET);
  bsb.WriteString(property_name);
}

void PersClientImpl::DelPropertyPrepare(const char *property_name, SerialBuffer& bsb) {
  bsb.WriteInt8(PC_DEL);
  bsb.WriteString(property_name);
}

void PersClientImpl::IncPropertyPrepare(Property& prop, SerialBuffer& bsb) {
  if(prop.getType() != INT && prop.getType() != DATE)
      throw PersClientException(INVALID_PROPERTY_TYPE);
  bsb.WriteInt8(PC_INC_RESULT);
  prop.Serialize(bsb);
}

void PersClientImpl::IncModPropertyPrepare(Property& prop, uint32_t mod, SerialBuffer& bsb) {
  if(prop.getType() != INT && prop.getType() != DATE)
      throw PersClientException(INVALID_PROPERTY_TYPE);
  bsb.WriteInt8(PC_INC_MOD);
  bsb.WriteInt32(mod);
  prop.Serialize(bsb);
}

void PersClientImpl::CheckServerResponse(SerialBuffer& bsb)
{
  switch (GetServerResponse(bsb)) {
  case RESPONSE_OK: break;
  case RESPONSE_PROPERTY_NOT_FOUND: throw PersClientException(PROPERTY_NOT_FOUND);
  case RESPONSE_ERROR: throw PersClientException(SERVER_ERROR);
  case RESPONSE_BAD_REQUEST: throw PersClientException(BAD_REQUEST);
  case RESPONSE_TYPE_INCONSISTENCE: throw PersClientException(TYPE_INCONSISTENCE);
  case RESPONSE_PROFILE_LOCKED: throw PersClientException(PROFILE_LOCKED);
  default: throw PersClientException(UNKNOWN_RESPONSE);
  }
}

int PersClientImpl::IncModPropertyResult(SerialBuffer& bsb)
{
  CheckServerResponse(bsb);
  
    //if(GetServerResponse(bsb) != RESPONSE_OK)
      //  throw PersClientException(SERVER_ERROR);

    try{
        return bsb.ReadInt32();
    }
    catch(SerialBufferOutOfBounds &e)
    {
        throw PersClientException(BAD_RESPONSE);
    }
}
int PersClientImpl::IncModProperty(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod)
{
    MutexGuard mt(mtx);

	emptyPacket(sb);
    IncModPropertyPrepare(pt, key, prop, mod, sb);
    SendPacket(sb);
    ReadPacket(sb);
    int i =  IncModPropertyResult(sb);
    actTS = time(NULL);
    return i;
}

void PersClientImpl::setBatchCount(uint32_t cnt, SerialBuffer& bsb)
{
	uint32_t i = bsb.GetPos();
	bsb.SetPos(5);
	bsb.WriteInt16(cnt);
	bsb.SetPos(i);
}

void PersClientImpl::PrepareMTBatch(SerialBuffer& bsb, ProfileType pt, const PersKey& key, uint16_t cnt, bool transactMode) {
  emptyPacket(bsb);
  FillHead(PC_MTBATCH, pt, key, bsb);
  bsb.WriteInt16(cnt);
  bsb.WriteInt8((uint8_t)transactMode);
}

void PersClientImpl::PrepareBatch(SerialBuffer& bsb, bool transactMode)
{
	bsb.setPos(4);
	transactMode ? bsb.WriteInt8(PC_TRANSACT_BATCH) : bsb.WriteInt8(PC_BATCH);
	bsb.WriteInt16(0);
}

void PersClientImpl::FinishPrepareBatch(uint32_t cnt, SerialBuffer& bsb)
{
	setBatchCount(cnt, bsb);
}

void PersClientImpl::RunBatch(SerialBuffer& bsb)
{
    MutexGuard mt(mtx);
    SendPacket(bsb);
    ReadPacket(bsb);
    actTS = time(NULL);
}

void PersClientImpl::Stop()
{
    smsc_log_info(log, "PersClient stopping...");
    isStopping = true;
    {
        MutexGuard mt(callMonitor), mt1(mtx);
        callMonitor.notify();
    }
    WaitFor();
    smsc_log_info(log, "PersClient stopped");
}

void PersClientImpl::init()
{
    char resp[3];
    
    if(connected) return;
        
    smsc_log_info(log, "Connecting to persserver host=%s:%d timeout=%d", host.c_str(), port, timeout);
    
    if(sock.Init(host.c_str(), port, timeout) == -1 || sock.Connect() == -1)
        throw PersClientException(CANT_CONNECT);
        
    if(sock.Read(resp, 2) != 2)
    {
        sock.Close();
        throw PersClientException(CANT_CONNECT);
    }
    resp[2] = 0;
    if(!strcmp(resp, "SB"))
    {
        sock.Close();
        throw PersClientException(SERVER_BUSY);
    } else if(strcmp(resp, "OK"))
    {
        sock.Close();
        throw PersClientException(UNKNOWN_RESPONSE);
    }
    connected = true;
}

void PersClientImpl::ReadAllTO(char* buf, uint32_t sz)
{
    int cnt;
    uint32_t rd = 0;
    struct pollfd pfd;

    while(sz)
    {
		pfd.fd = sock.getSocket();
		pfd.events = POLLIN;
        if(poll(&pfd, 1, timeout * 1000) <= 0 || !(pfd.revents & POLLIN))
            throw PersClientException(TIMEOUT);

        cnt = sock.Read(buf + rd, sz);
        if(cnt <= 0)
            throw PersClientException(READ_FAILED);

        rd += cnt;
        sz -= cnt;
    }
}

void PersClientImpl::WriteAllTO(const char* buf, uint32_t sz)
{
    int cnt;
    uint32_t wr = 0;
    struct pollfd pfd;    

    while(sz)
    {
		pfd.fd = sock.getSocket();
		pfd.events = POLLOUT | POLLIN;
        if(poll(&pfd, 1, timeout * 1000) <= 0)
            throw PersClientException(TIMEOUT);
		else if(!(pfd.revents & POLLOUT) || (pfd.revents & POLLIN))
            throw PersClientException(SEND_FAILED);		

        cnt = sock.Write(buf + wr, sz);
        if(cnt <= 0)
            throw PersClientException(SEND_FAILED);

        wr += cnt;
        sz -= cnt;
    }
}

void PersClientImpl::SendPacket(SerialBuffer& bsb)
{
    uint32_t  t = 0;
    
//    if(!connected)
//        throw PersClientException(NOT_CONNECTED);
    
    for(;;)
    {
        try{
            init();
            setPacketSize(bsb);
            WriteAllTO(bsb.c_ptr(), bsb.length());
            smsc_log_debug(log, "write to socket: len=%d, data=%s", bsb.length(), bsb.toString().c_str());			
            return;
        }
        catch(PersClientException &e)
        {
            smsc_log_debug(log, "PersClientException: %s", e.what());		
            connected = false;
            sock.Close();
            if(++t >= 2) throw;
        }
    }
}

void PersClientImpl::ReadPacket(SerialBuffer& bsb)
{
    char tmp_buf[1024];
    uint32_t sz;

    if(!connected)
        throw PersClientException(NOT_CONNECTED);

    bsb.Empty();
    ReadAllTO(tmp_buf, sizeof(uint32_t));
    bsb.Append(tmp_buf, sizeof(uint32_t));
    bsb.SetPos(0);
    sz = bsb.ReadInt32() - sizeof(uint32_t);
    smsc_log_debug(log, "%d bytes will be read from socket", sz);

    while(sz > 1024)
    {
        ReadAllTO(tmp_buf, 1024);
        bsb.Append(tmp_buf, 1024);
        sz -= 1024;
    }
    if(sz)
    {
        ReadAllTO(tmp_buf, sz);
        bsb.Append(tmp_buf, sz);
    }
    smsc_log_debug(log, "read from socket: len=%d, data=%s", bsb.length(), bsb.toString().c_str());
	bsb.SetPos(4);
}

void PersClientImpl::setPacketSize(SerialBuffer& bsb)
{
    bsb.SetPos(0);
    bsb.WriteInt32(bsb.GetSize());
}

uint32_t PersClientImpl::getPacketSize(SerialBuffer& bsb)
{
    bsb.SetPos(0);
    return bsb.ReadInt32();
}

PersServerResponseType PersClientImpl::GetServerResponse(SerialBuffer& bsb)
{
    try{
        return (PersServerResponseType)bsb.ReadInt8();
    }
    catch(SerialBufferOutOfBounds &e)
    {
        throw PersClientException(BAD_RESPONSE);
    }
}

void PersClientImpl::FillHead(PersCmd pc, ProfileType pt, const PersKey& key, SerialBuffer& bsb)
{
    bsb.WriteInt8((uint8_t)pc);
    bsb.WriteInt8((uint8_t)pt);
	if(pt == PT_ABONENT)
	    bsb.WriteString(key.skey);
	else
	    bsb.WriteInt32(key.ikey);	
}

void PersClientImpl::ParseProperty(Property& prop, SerialBuffer& bsb)
{
    try{
	    PersServerResponseType ss = (PersServerResponseType)bsb.ReadInt8();
		
	    if(ss == RESPONSE_PROPERTY_NOT_FOUND)
    	    throw PersClientException(PROPERTY_NOT_FOUND);

	    if(ss != RESPONSE_OK)
    	    throw PersClientException(SERVER_ERROR);
	
        prop.Deserialize(bsb);
    } catch(SerialBufferOutOfBounds &e)
    {
        throw PersClientException(BAD_RESPONSE);
    }
}

LongCallContext* PersClientImpl::getContext()
{
    LongCallContext *ctx;
    MutexGuard mt(callMonitor);
    if(isStopping) return NULL;
    if(!headContext) callMonitor.wait(pingTimeout * 1000);
    ctx = headContext;    
    if(headContext) headContext = headContext->next;
    return ctx;        
}

bool PersClientImpl::call(LongCallContext* context)
{
    MutexGuard mt(callMonitor);
    if(isStopping) return false;
    context->next = NULL;

    if(headContext)
        tailContext->next = context;
    else
        headContext = context;
    tailContext = context;
    
    callMonitor.notify();
    return true;
}

void PersClientImpl::ping()
{
    MutexGuard mt(mtx);
    try{
        sb.Empty();
        sb.SetPos(4);
        sb.WriteInt8(PC_PING);
        SendPacket(sb);
        ReadPacket(sb);
        if(GetServerResponse(sb) != RESPONSE_OK)
            throw PersClientException(SERVER_ERROR);
        smsc_log_debug(log, "Ping sent");
    }
    catch(PersClientException& e)
    {
        smsc_log_error(log, "Ping failed: %s", e.what());
        if(connected)
        {
            sock.Close();
            connected = false;
        }
        try{
            init();
        }
        catch(...)
        {};
    }
    catch(...)
    {
        smsc_log_error(log, "Unknown exception");
    }
}

void PersClientImpl::ExecutePersCall(LongCallContext* ctx)
{
    PersCallParams* persParams = (PersCallParams*)ctx->getParams();
    smsc_log_debug(log, "ExecutePersCall: command=%d %s", ctx->callCommandId, persParams->skey.c_str());
    try{
        //persParams->error = 0;
        //persParams->exception.assign("");
		if(ctx->callCommandId == PERS_BATCH) {
          if (persParams->error == 0) {
            RunBatch(persParams->sb);
          }
        }
		else if(persParams->pt == PT_ABONENT)   {     
          persParams->error = 0;
          persParams->exception.assign("");
          switch(ctx->callCommandId)
          {
              case PERS_GET: GetProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str(), persParams->prop); break;
              case PERS_SET: SetProperty(persParams->pt, persParams->skey.c_str(), persParams->prop); break;
              //case PERS_DEL: if(!DelProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str())) persParams->error = PROPERTY_NOT_FOUND; break;
              case PERS_DEL: DelProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str());  break;
              case PERS_INC_MOD: persParams->result = IncModProperty(persParams->pt, persParams->skey.c_str(), persParams->prop, persParams->mod); break;
              case PERS_INC: persParams->result = IncProperty(persParams->pt, persParams->skey.c_str(), persParams->prop); break;
          }
        }
        else {
          persParams->error = 0;
          persParams->exception.assign("");
          switch(ctx->callCommandId)
          {
              case PERS_GET: GetProperty(persParams->pt, persParams->ikey, persParams->propName.c_str(), persParams->prop); break;
              case PERS_SET: SetProperty(persParams->pt, persParams->ikey, persParams->prop); break;
              //case PERS_DEL: if(!DelProperty(persParams->pt, persParams->ikey, persParams->propName.c_str())) persParams->error = PROPERTY_NOT_FOUND; break;
              case PERS_DEL: DelProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str());  break;
              case PERS_INC_MOD: persParams->result = IncModProperty(persParams->pt, persParams->ikey, persParams->prop, persParams->mod); break;
              case PERS_INC: persParams->result = IncProperty(persParams->pt, persParams->ikey, persParams->prop); break;
          }
        }
    }
    catch(PersClientException& exc)
    {
        persParams->error = exc.getType();
        persParams->exception = exc.what();        
    }
    catch(Exception& exc)
    {
        persParams->error = -1;        
        persParams->exception = exc.what();
    }
    catch(...)
    {
        persParams->error = -1;        
        persParams->exception = "LongCallManager: Unknown exception";
    }
}

int PersClientImpl::Execute()
{
    smsc_log_debug(log, "Pers thread started");
    
    while(!isStopping)
    {
        LongCallContext* ctx = getContext();
        
        if(isStopping) break;

        if(ctx)
        {
            ExecutePersCall(ctx);
                
            ctx->initiator->continueExecution(ctx, false);

            actTS = time(NULL);
        }
        else
        {
            if(actTS + pingTimeout < time(NULL))
            {
                ping();
                actTS = time(NULL);
            }
        }
    }

    MutexGuard mg(callMonitor);
    while(headContext)
    {
        LongCallContext* ctx = headContext;
        headContext = headContext->next;
        ctx->initiator->continueExecution(ctx, true);
    }
    
    smsc_log_debug(log, "Pers thread finished");
    return 0;
}

}}}
