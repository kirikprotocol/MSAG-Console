/* $Id$ */

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
using scag::pers;

bool  PersClient::inited = false;
Mutex PersClient::initLock;

class PersClientImpl: public PersClient, public scag::config::ConfigListener, public Thread {
//    friend class PersClient;
public:
    PersClientImpl(): connected(false), headContext(NULL), ConfigListener(scag::config::PERSCLIENT_CFG) {};
    ~PersClientImpl() { Stop(); if(connected) sock.Close(); };

    void SetProperty(ProfileType pt, const char* key, Property& prop);// throw(PersClientException);
    void SetProperty(ProfileType pt, uint32_t key, Property& prop);// throw(PersClientException);

    void GetProperty(ProfileType pt, const char* key, const char *property_name, Property& prop);// throw(PersClientException);
    void GetProperty(ProfileType pt, uint32_t key, const char *property_name, Property& prop);// throw(PersClientException);

    void DelProperty(ProfileType pt, const char* key, const char *property_name);// throw(PersClientException);
    void DelProperty(ProfileType pt, uint32_t key, const char *property_name);// throw(PersClientException);

    void IncProperty(ProfileType pt, const char* key, Property& prop);// throw(PersClientException);
    void IncProperty(ProfileType pt, uint32_t key, Property& prop);// throw(PersClientException);

    int IncModProperty(ProfileType pt, const char* key, Property& prop, uint32_t mod); //throw(PersClientException)
    int IncModProperty(ProfileType pt, uint32_t key, Property& prop, uint32_t mod); //throw(PersClientException)

    bool call(LongCallContext* context);

    void init_internal(const char *_host, int _port, int timeout, int _pingTimeout); //throw(PersClientException);
    
    virtual int Execute();
    void Stop() { isStopping = true; {MutexGuard mt(callMutex), mt1(mtx); callMutex.notify();} WaitFor();};
    void Start() {isStopping = false; Thread::Start();};
    
protected:

    void configChanged();
    void init();
    void reinit(const char *_host, int _port, int _timeout, int _pingTimeout); //throw(PersClientException)    
    void SetPacketSize();
    void _SetProperty(ProfileType pt, const char* skey, uint32_t ikey, Property& prop); //throw(PersClientException)
    void _DelProperty(ProfileType pt, const char* skey, uint32_t ikey, const char *property_name); //throw(PersClientException)
    void _IncProperty(ProfileType pt, const char* key, uint32_t ikey, Property& prop); //throw(PersClientException)
    int _IncModProperty(ProfileType pt, const char* key, uint32_t ikey, Property& prop, uint32_t mod); //throw(PersClientException)
    void FillHead(PersCmd pc, ProfileType pt, const char *key);
    void FillHead(PersCmd pc, ProfileType pt, uint32_t key);
    void AppendString(const char *str);
    void SendPacket();
    void ReadPacket();
    void WriteAllTO(const char* buf, uint32_t sz);
    void ReadAllTO(char* buf, uint32_t sz);
    uint32_t GetPacketSize();
    PersServerResponseType GetServerResponse();
    void ParseProperty(Property& prop);

    LongCallContext* getContext();
    void ping();
    void ExecutePersCall(LongCallContext* ctx);

    std::string host;
    int port;
    int timeout, pingTimeout;
    time_t actTS;
    Socket sock;
    bool connected, isStopping;
    Logger * log;
    Mutex mtx;
    EventMonitor callMutex;
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

void PersClientImpl::_SetProperty(ProfileType pt, const char* skey, uint32_t ikey, Property& prop) //throw(PersClientException)
{
    MutexGuard mt(mtx);

    if(pt == PT_ABONENT)
        FillHead(PC_SET, pt, skey);
    else
        FillHead(PC_SET, pt, ikey);
    prop.Serialize(sb);
    SendPacket();
    ReadPacket();
    if(GetServerResponse() != RESPONSE_OK)
        throw PersClientException(SERVER_ERROR);
        
    actTS = time(NULL);
}

void PersClientImpl::SetProperty(ProfileType pt, const char* key, Property& prop) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _SetProperty(pt, key, 0, prop);
}
void PersClientImpl::SetProperty(ProfileType pt, uint32_t key, Property& prop) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _SetProperty(pt, NULL, key, prop);
}

void PersClientImpl::GetProperty(ProfileType pt, const char* key, const char *property_name, Property& prop) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);


    MutexGuard mt(mtx);

    FillHead(PC_GET, pt, key);
    sb.WriteString(property_name);
    SendPacket();
    ReadPacket();
    ParseProperty(prop);
    actTS = time(NULL);    
}
void PersClientImpl::GetProperty(ProfileType pt, uint32_t key, const char *property_name, Property& prop) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    MutexGuard mt(mtx);

    FillHead(PC_GET, pt, key);
    sb.WriteString(property_name);
    SendPacket();
    ReadPacket();
    ParseProperty(prop);
    actTS = time(NULL);    
}

void PersClientImpl::_DelProperty(ProfileType pt, const char* skey, uint32_t ikey, const char *property_name) //throw(PersClientException)
{
    MutexGuard mt(mtx);

    if(pt == PT_ABONENT)
        FillHead(PC_DEL, pt, skey);
    else
        FillHead(PC_DEL, pt, ikey);

    sb.WriteString(property_name);
    SendPacket();
    ReadPacket();

    int resp = GetServerResponse();

    if(resp == RESPONSE_PROPERTY_NOT_FOUND)
        throw PersClientException(PROPERTY_NOT_FOUND);
    else if(resp != RESPONSE_OK)
        throw PersClientException(SERVER_ERROR);
    actTS = time(NULL);        
}

void PersClientImpl::DelProperty(ProfileType pt, const char* key, const char *property_name) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _DelProperty(pt, key, 0, property_name);
}

void PersClientImpl::DelProperty(ProfileType pt, uint32_t key, const char *property_name) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _DelProperty(pt, NULL, key, property_name);
}

void PersClientImpl::_IncProperty(ProfileType pt, const char* skey, uint32_t ikey, Property& prop) //throw(PersClientException)
{
    if(prop.getType() != INT && prop.getType() != DATE)
        throw PersClientException(INVALID_PROPERTY_TYPE);

    MutexGuard mt(mtx);

    if(pt == PT_ABONENT)
        FillHead(PC_INC, pt, skey);
    else
        FillHead(PC_INC, pt, ikey);

    prop.Serialize(sb);
    SendPacket();
    ReadPacket();
    if(GetServerResponse() != RESPONSE_OK)
        throw PersClientException(SERVER_ERROR);
    actTS = time(NULL);        
}
void PersClientImpl::IncProperty(ProfileType pt, const char* key, Property& prop) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _IncProperty(pt, key, 0, prop);
}
void PersClientImpl::IncProperty(ProfileType pt, uint32_t key, Property& prop) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _IncProperty(pt, NULL, key, prop);
}

int PersClientImpl::_IncModProperty(ProfileType pt, const char* skey, uint32_t ikey, Property& prop, uint32_t mod) //throw(PersClientException)
{
    if(prop.getType() != INT && prop.getType() != DATE)
        throw PersClientException(INVALID_PROPERTY_TYPE);

    MutexGuard mt(mtx);

    if(pt == PT_ABONENT)
        FillHead(PC_INC_MOD, pt, skey);
    else
        FillHead(PC_INC_MOD, pt, ikey);

    sb.WriteInt32(mod);
    prop.Serialize(sb);
    SendPacket();
    ReadPacket();

    if(GetServerResponse() != RESPONSE_OK)
        throw PersClientException(SERVER_ERROR);

    try{
        sb.SetPos(sizeof(uint32_t) + 1);
        actTS = time(NULL);        
        return sb.ReadInt32();
    }
    catch(SerialBufferOutOfBounds &e)
    {
        throw PersClientException(BAD_RESPONSE);
    }

}
int PersClientImpl::IncModProperty(ProfileType pt, const char* key, Property& prop, uint32_t mod) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    return _IncModProperty(pt, key, 0, prop, mod);
}
int PersClientImpl::IncModProperty(ProfileType pt, uint32_t key, Property& prop, uint32_t mod) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    return _IncModProperty(pt, NULL, key, prop, mod);
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

    while(sz)
    {
        if(sock.canRead(0) <= 0)
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

    while(sz)
    {
        if(sock.canWrite(0) <= 0)
            throw PersClientException(TIMEOUT);

        cnt = sock.Write(buf + wr, sz);
        if(cnt <= 0)
            throw PersClientException(SEND_FAILED);

        wr += cnt;
        sz -= cnt;
    }
}

void PersClientImpl::SendPacket()
{
    uint32_t  t = 0;
    for(;;)
    {
        try{
            init();            
            SetPacketSize();
            WriteAllTO(sb.c_ptr(), sb.length());
            return;
        }
        catch(PersClientException &e)
        {
            connected = false;
            sock.Close();
            if(++t >= 2) throw e;
        }
    }
}

void PersClientImpl::ReadPacket()
{
    char tmp_buf[1024];
    uint32_t sz;

    if(!connected)
        throw PersClientException(NOT_CONNECTED);

    sb.Empty();
    ReadAllTO(tmp_buf, sizeof(uint32_t));
    sb.Append(tmp_buf, sizeof(uint32_t));
    sb.SetPos(0);
    sz = sb.ReadInt32() - sizeof(uint32_t);

    while(sz > 1024)
    {
        ReadAllTO(tmp_buf, 1024);
        sb.Append(tmp_buf, 1024);
        sz -= 1024;
    }
    if(sz)
    {
        ReadAllTO(tmp_buf, sz);
        sb.Append(tmp_buf, sz);
    }
}

void PersClientImpl::SetPacketSize()
{
    sb.SetPos(0);
    sb.WriteInt32(sb.GetSize());
}

uint32_t PersClientImpl::GetPacketSize()
{
    sb.SetPos(0);
    return sb.ReadInt32();
}

PersServerResponseType PersClientImpl::GetServerResponse()
{
    try{
        sb.SetPos(sizeof(uint32_t));
        return (PersServerResponseType)sb.ReadInt8();
    }
    catch(SerialBufferOutOfBounds &e)
    {
        throw PersClientException(BAD_RESPONSE);
    }
}

void PersClientImpl::FillHead(PersCmd pc, ProfileType pt, uint32_t key)
{
    sb.Empty();
    sb.SetPos(4);
    sb.WriteInt8((uint8_t)pc);
    sb.WriteInt8((uint8_t)pt);
    sb.WriteInt32(key);
}

void PersClientImpl::FillHead(PersCmd pc, ProfileType pt, const char *key)
{
    sb.Empty();
    sb.SetPos(4);
    sb.WriteInt8((uint8_t)pc);
    sb.WriteInt8((uint8_t)pt);
    sb.WriteString(key);
}

void PersClientImpl::ParseProperty(Property& prop)
{
    PersServerResponseType ss = GetServerResponse();
    if(ss == RESPONSE_PROPERTY_NOT_FOUND)
        throw PersClientException(PROPERTY_NOT_FOUND);

    if(ss != RESPONSE_OK)
        throw PersClientException(SERVER_ERROR);


    sb.SetPos(sizeof(uint32_t) + 1);
    try{
        prop.Deserialize(sb);
    } catch(SerialBufferOutOfBounds &e)
    {
        throw PersClientException(BAD_RESPONSE);
    }
}

LongCallContext* PersClientImpl::getContext()
{
    LongCallContext *ctx;
    MutexGuard mt(callMutex);
    if(isStopping) return NULL;
    if(!headContext) callMutex.wait(pingTimeout * 1000);
    ctx = headContext;    
    if(headContext) headContext = headContext->next;
    return ctx;        
}

bool PersClientImpl::call(LongCallContext* context)
{
    MutexGuard mt(callMutex);
    if(isStopping) return false;
    context->next = NULL;

    if(headContext)
        tailContext->next = context;
    else
        headContext = context;
    tailContext = context;
    
    callMutex.notify();        
}

void PersClientImpl::ping()
{
    MutexGuard mt(mtx);
    try{
        sb.Empty();
        sb.SetPos(4);
        sb.WriteInt8(PC_PING);
        SendPacket();
        ReadPacket();
        if(GetServerResponse() != RESPONSE_OK)
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
        if(persParams->pt == PT_ABONENT)        
            switch(ctx->callCommandId)
            {
                case PERS_GET: GetProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str(), persParams->prop); break;
                case PERS_SET: SetProperty(persParams->pt, persParams->skey.c_str(), persParams->prop); break;
                case PERS_DEL: DelProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str()); break;
                case PERS_INC_MOD: persParams->result = IncModProperty(persParams->pt, persParams->skey.c_str(), persParams->prop, persParams->mod); break;
                case PERS_INC: IncProperty(persParams->pt, persParams->skey.c_str(), persParams->prop); break;
            }
        else
            switch(ctx->callCommandId)
            {
                case PERS_GET: GetProperty(persParams->pt, persParams->ikey, persParams->propName.c_str(), persParams->prop); break;
                case PERS_SET: SetProperty(persParams->pt, persParams->ikey, persParams->prop); break;
                case PERS_DEL: DelProperty(persParams->pt, persParams->ikey, persParams->propName.c_str()); break;
                case PERS_INC_MOD: persParams->result = IncModProperty(persParams->pt, persParams->ikey, persParams->prop, persParams->mod); break;
                case PERS_INC: IncProperty(persParams->pt, persParams->ikey, persParams->prop); break;
            }
        persParams->error = 0;
        persParams->exception.assign("");
    }
    catch(PersClientException& exc)
    {
        persParams->error = exc.getType();
        persParams->exception = exc.what();        
    }
    catch(Exception& exc)
    {
        persParams->error = 0;        
        persParams->exception = exc.what();
    }
    catch(...)
    {
        persParams->error = 0;        
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
            if(ctx->callCommandId >= PERS_GET && ctx->callCommandId <= PERS_INC_MOD)
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

    smsc_log_debug(log, "Pers thread finished");
    return 0;
}

}}}
