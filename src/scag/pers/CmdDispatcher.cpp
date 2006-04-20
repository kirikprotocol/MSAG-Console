/* $Id$ */

#include "CmdDispatcher.h"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::logger::Logger;

CommandDispatcher::CommandDispatcher(StringProfileStore *abonent, IntProfileStore *service, IntProfileStore *oper, IntProfileStore *provider):
        log(Logger::getInstance("cmd"))
{
    int_store[0].pt = PT_SERVICE;
    int_store[1].pt = PT_OPERATOR;
    int_store[2].pt = PT_PROVIDER;
    int_store[0].store = service;
    int_store[1].store = oper;
    int_store[2].store = provider;
    AbonentStore = abonent;
}

void CommandDispatcher::SendResponse(SerialBuffer *sb, PersServerResponseType r)
{
    sb->Empty();
    sb->WriteInt32(5);
    sb->WriteInt8((uint8_t)r);
}

void CommandDispatcher::SetPacketSize(SerialBuffer *sb)
{
    sb->SetPos(0);
    sb->WriteInt32(sb->GetSize());
}

void CommandDispatcher::DelCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, std::string& name, SerialBuffer *sb)
{
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "DelCmdHandler AbonetStore: key=%s, name=%s", str_key.c_str(), name.c_str());
        exists = AbonentStore->delProperty(StringProfileKey(str_key.c_str()), name.c_str());
    }
    else
        for(int i = 0; i < INT_STORE_CNT; i++)
            if(pt == int_store[i].pt)
            {
                smsc_log_debug(log, "DelCmdHandler store= %d, key=%d, name=%s", pt, int_key, name.c_str());
                exists = int_store[i].store->delProperty(IntProfileKey(int_key), name.c_str());
            }
    SendResponse(sb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::GetCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, std::string& name, SerialBuffer *sb)
{
    Property prop;
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "GetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), name.c_str());
        exists = AbonentStore->getProperty(StringProfileKey(str_key.c_str()), name.c_str(), prop);
    }
    else
        for(int i = 0; i < INT_STORE_CNT; i++)
            if(pt == int_store[i].pt)
            {
                smsc_log_debug(log, "GetCmdHandler store=%d, key=%d, name=%s", pt, int_key, name.c_str());
                exists = int_store[i].store->getProperty(IntProfileKey(int_key), name.c_str(), prop);
            }
    if(exists)
    {   
        smsc_log_debug(log, "GetCmdHandler prop=%s", prop.toString().c_str());
        SendResponse(sb, RESPONSE_OK);
        prop.Serialize(*sb);
        SetPacketSize(sb);
    } else
        SendResponse(sb, RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::SetCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, Property& prop, SerialBuffer *sb)
{
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "SetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.toString().c_str());
        AbonentStore->setProperty(StringProfileKey(str_key.c_str()), prop);
    }
    else
        for(int i = 0; i < INT_STORE_CNT; i++)
            if(pt == int_store[i].pt)
            {
                smsc_log_debug(log, "SetCmdHandler store=%d, key=%d, prop=%s", pt, int_key, prop.toString().c_str());
                int_store[i].store->setProperty(IntProfileKey(int_key), prop);
            }
    SendResponse(sb, RESPONSE_OK);
}

void CommandDispatcher::IncCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, Property& prop, SerialBuffer *sb)
{
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName().c_str());
        exists = AbonentStore->incProperty(StringProfileKey(str_key.c_str()), prop);
    }
    else
        for(int i = 0; i < INT_STORE_CNT; i++)
            if(pt == int_store[i].pt)
            {
                smsc_log_debug(log, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName().c_str());
                exists = int_store[i].store->incProperty(IntProfileKey(int_key), prop);
            }
    if(exists)
        SendResponse(sb, RESPONSE_OK);
    else
        SendResponse(sb, RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::IncModCmdHandler(ProfileType pt, uint32_t int_key, std::string& str_key, Property& prop, int mod, SerialBuffer *sb)
{
    bool exists = false;
    int res = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "IncModCmdHandler AbonentStore: key=%s, name=%s, mod=%d", str_key.c_str(), prop.getName().c_str(), mod);
        exists = AbonentStore->incModProperty(StringProfileKey(str_key.c_str()), prop, mod, res);
    }
    else
        for(int i = 0; i < INT_STORE_CNT; i++)
            if(pt == int_store[i].pt)
            {
                smsc_log_debug(log, "IncCmdHandler store=%d, key=%d, name=%s, mod=%d", pt, int_key, prop.getName().c_str(), mod);
                exists = int_store[i].store->incModProperty(IntProfileKey(int_key), prop, mod, res);
            }
    if(exists)
    {
        SendResponse(sb, RESPONSE_OK);
        sb->WriteInt32(res);
        SetPacketSize(sb);
    }
    else
        SendResponse(sb, RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::Execute(SerialBuffer* sb)
{
    PersCmd cmd;
    ProfileType pt;
    uint32_t int_key;
    string str_key;
    string name;
    Property prop;

    try{
        sb->SetPos(4);
        cmd = (PersCmd)sb->ReadInt8();
        pt = (ProfileType)sb->ReadInt8();
        if(pt != PT_ABONENT)
            int_key = sb->ReadInt32();
        else
            sb->ReadString(str_key);
        switch(cmd)
        {
            case PC_DEL:
                sb->ReadString(name);
                DelCmdHandler(pt, int_key, str_key, name, sb);
                return;
            case PC_SET:
                prop.Deserialize(*sb);
                SetCmdHandler(pt, int_key, str_key, prop, sb);
                return;
            case PC_GET:
                sb->ReadString(name);
                GetCmdHandler(pt, int_key, str_key, name, sb);
                return;
            case PC_INC:
                prop.Deserialize(*sb);
                IncCmdHandler(pt, int_key, str_key, prop, sb);
                return;
            case PC_INC_MOD:
                int inc = sb->ReadInt32();
                prop.Deserialize(*sb);
                IncModCmdHandler(pt, int_key, str_key, prop, inc, sb);
                return;
        }
    }
    catch(SerialBufferOutOfBounds &e)
    {
    }
    SendResponse(sb, RESPONSE_BAD_REQUEST);
}

}}
