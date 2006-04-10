/* $Id$ */

#include <scag/util/singleton/Singleton.h>

#include "PersClient.h"

namespace scag { namespace pers { namespace client {

using namespace scag::util::singleton;
using smsc::util::Exception;
using smsc::logger::Logger;
using scag::pers;

bool  PersClient::inited = false;
Mutex PersClient::initLock;

inline unsigned GetLongevity(PersClient*) { return 5; }
typedef SingletonHolder<PersClient> SinglePC;

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

void PersClient::Init(const char *_host, int _port, int _timeout) //throw(PersClientException)
{
    if (!PersClient::inited)
    {
        MutexGuard guard(PersClient::initLock);
        if(!PersClient::inited) {
            PersClient& pc = SinglePC::Instance();
            pc.init_internal(_host, _port, _timeout);
            PersClient::inited = true;
        }
    }
}

void PersClient::init_internal(const char *_host, int _port, int _timeout) //throw(PersClientException)
{
    log = Logger::getInstance("client");
    connected = false;
    host = _host;
    port = _port;
    timeout = _timeout;
    init();
}

void PersClient::_SetProperty(ProfileType pt, const char* skey, uint32_t ikey, Property& prop) //throw(PersClientException)
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
}

void PersClient::SetProperty(ProfileType pt, const char* key, Property& prop) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _SetProperty(pt, key, 0, prop);
}
void PersClient::SetProperty(ProfileType pt, uint32_t key, Property& prop) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    _SetProperty(pt, NULL, key, prop);
}

void PersClient::GetProperty(ProfileType pt, const char* key, const char *property_name, Property& prop) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);


    MutexGuard mt(mtx);

    FillHead(PC_GET, pt, key);
    sb.WriteString(property_name);
    SendPacket();
    ReadPacket();
    ParseProperty(prop);
}
void PersClient::GetProperty(ProfileType pt, uint32_t key, const char *property_name, Property& prop) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    MutexGuard mt(mtx);

    FillHead(PC_GET, pt, key);
    sb.WriteString(property_name);
    SendPacket();
    ReadPacket();
    ParseProperty(prop);
}

void PersClient::DelProperty(ProfileType pt, const char* key, const char *property_name) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    MutexGuard mt(mtx);

    FillHead(PC_DEL, pt, key);
    sb.WriteString(property_name);
    SendPacket();
    ReadPacket();
    if(GetServerResponse() == RESPONSE_ERROR)
        throw PersClientException(SERVER_ERROR);
}
void PersClient::DelProperty(ProfileType pt, uint32_t key, const char *property_name) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    MutexGuard mt(mtx);

    FillHead(PC_DEL, pt, key);
    sb.WriteString(property_name);
    SendPacket();
    ReadPacket();
    if(GetServerResponse() == RESPONSE_ERROR)
        throw PersClientException(SERVER_ERROR);
}

void PersClient::IncProperty(ProfileType pt, const char* key, const char *property_name, int32_t inc) //throw(PersClientException)
{
    if(pt != PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    MutexGuard mt(mtx);

    FillHead(PC_INC, pt, key);
    sb.WriteString(property_name);
    sb.WriteInt32(inc);
    SendPacket();
    ReadPacket();
    if(GetServerResponse() != RESPONSE_OK)
        throw PersClientException(PROPERTY_NOT_FOUND);
}
void PersClient::IncProperty(ProfileType pt, uint32_t key, const char *property_name, int32_t inc) //throw(PersClientException)
{
    if(pt == PT_ABONENT)
        throw PersClientException(INVALID_KEY);

    MutexGuard mt(mtx);

    FillHead(PC_INC, pt, key);
    sb.WriteString(property_name);
    sb.WriteInt32((uint32_t)inc);
    SendPacket();
    ReadPacket();
    if(GetServerResponse() != RESPONSE_OK)
        throw PersClientException(PROPERTY_NOT_FOUND);
}

void PersClient::init()
{
    char resp[3];
    if(connected)
        return;
    if(sock.Init(host.c_str(), port, timeout) == -1)
        throw PersClientException(CANT_CONNECT);
    if(sock.Connect() == -1)
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

void PersClient::ReadAllTO(char* buf, uint32_t sz)
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

void PersClient::WriteAllTO(char* buf, uint32_t sz)
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

void PersClient::SendPacket()
{
    init();
    try{
        SetPacketSize();
        WriteAllTO(sb.get(), sb.GetSize());
    }
    catch(PersClientException &e)
    {
        connected = false;
        sock.Close();
        throw e;
    }
}

void PersClient::ReadPacket()
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

void PersClient::SetPacketSize()
{
    sb.SetPos(0);
    sb.WriteInt32(sb.GetSize());
}

uint32_t PersClient::GetPacketSize()
{
    sb.SetPos(0);
    return sb.ReadInt32();
}

PersServerResponseType PersClient::GetServerResponse()
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

void PersClient::FillHead(PersCmd pc, ProfileType pt, uint32_t key)
{
    sb.Empty();
    sb.SetPos(4);
    sb.WriteInt8((uint8_t)pc);
    sb.WriteInt8((uint8_t)pt);
    sb.WriteInt32(key);
}

void PersClient::FillHead(PersCmd pc, ProfileType pt, const char *key)
{
    sb.Empty();
    sb.SetPos(4);
    sb.WriteInt8((uint8_t)pc);
    sb.WriteInt8((uint8_t)pt);
    sb.WriteString(key);
}

void PersClient::ParseProperty(Property& prop)
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

}}}
