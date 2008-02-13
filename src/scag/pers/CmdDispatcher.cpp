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

IntProfileStore* CommandDispatcher::findStore(ProfileType pt)
{
    int i = 0;
    while(i < INT_STORE_CNT && pt != int_store[i].pt) i++;
    return i < INT_STORE_CNT ? int_store[i].store : NULL;            
}

void CommandDispatcher::SendResponse(SerialBuffer& sb, PersServerResponseType r)
{
    sb.WriteInt8((uint8_t)r);
}

void CommandDispatcher::SetPacketSize(SerialBuffer& sb)
{
    sb.SetPos(0);
    sb.WriteInt32(sb.GetSize());
}

void CommandDispatcher::DelCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb)
{
    IntProfileStore *is;
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "DelCmdHandler AbonetStore: key=%s, name=%s", str_key.c_str(), name.c_str());
        exists = AbonentStore->delProperty(str_key.c_str(), name.c_str());
    }
    else if(is = findStore(pt))
    {
        smsc_log_debug(log, "DelCmdHandler store= %d, key=%d, name=%s", pt, int_key, name.c_str());
        exists = is->delProperty(int_key, name.c_str());
    }
    SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::GetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb)
{
    IntProfileStore *is;
    Property prop;
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "GetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), name.c_str());
        exists = AbonentStore->getProperty(str_key.c_str(), name.c_str(), prop);
    }
    else if(is = findStore(pt))
    {
        smsc_log_debug(log, "GetCmdHandler store=%d, key=%d, name=%s", pt, int_key, name.c_str());
        exists = is->getProperty(int_key, name.c_str(), prop);
    }
    if(exists)
    {   
        smsc_log_debug(log, "GetCmdHandler prop=%s", prop.toString().c_str());
        SendResponse(osb, RESPONSE_OK);
        prop.Serialize(osb);
    }
    else
    {
        smsc_log_debug(log, "GetCmdHandler property not found: store=%d, key=%s(%d), name=%s", pt, str_key.c_str(), int_key, name.c_str());    
        SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
    }        
}

void CommandDispatcher::SetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "SetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.toString().c_str());
        AbonentStore->setProperty(str_key.c_str(), prop);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(log, "SetCmdHandler store=%d, key=%d, prop=%s", pt, int_key, prop.toString().c_str());
        is->setProperty(int_key, prop);
    }
    SendResponse(osb, RESPONSE_OK);
}

void CommandDispatcher::IncCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    bool exists = false;
    int result = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName().c_str());
        exists = AbonentStore->incProperty(str_key.c_str(), prop, result);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(log, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName().c_str());
        exists = is->incProperty(int_key, prop, result);
    }
    SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
    //osb.WriteInt32(result);
}

void CommandDispatcher::IncResultCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    bool exists = false;
    int result = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName().c_str());
        exists = AbonentStore->incProperty(str_key.c_str(), prop, result);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(log, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName().c_str());
        exists = is->incProperty(int_key, prop, result);
    }
    if(exists)
    {
        SendResponse(osb, RESPONSE_OK);
        osb.WriteInt32(result);
    }
    else
        SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::IncModCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, int mod, SerialBuffer& osb)
{
    IntProfileStore *is;
    bool exists = false;
    int res = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(log, "IncModCmdHandler AbonentStore: key=%s, name=%s, mod=%d", str_key.c_str(), prop.getName().c_str(), mod);
        exists = AbonentStore->incModProperty(str_key.c_str(), prop, mod, res);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(log, "IncModCmdHandler store=%d, key=%d, name=%s, mod=%d", pt, int_key, prop.getName().c_str(), mod);
        exists = is->incModProperty(int_key, prop, mod, res);
    }
    if(exists)
    {
        SendResponse(osb, RESPONSE_OK);
        osb.WriteInt32(res);
    }
    else
        SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
}

void CommandDispatcher::Execute(SerialBuffer& isb, SerialBuffer& osb)
{
    osb.SetPos(4);
    try{
		execCommand(isb, osb);
		SetPacketSize(osb);
		return;
    }
    catch(SerialBufferOutOfBounds &e)
    {
        smsc_log_debug(log, "Bad data in buffer received len=%d, data=%s", isb.length(), isb.toString().c_str());
    }
    catch(...)
    {
        smsc_log_debug(log, "Bad data in buffer received len=%d, data=%s", isb.length(), isb.toString().c_str());
    }
    SendResponse(osb, RESPONSE_BAD_REQUEST);
	SetPacketSize(osb);	
}

void CommandDispatcher::execCommand(SerialBuffer& isb, SerialBuffer& osb)
{
    PersCmd cmd;
    ProfileType pt;
    uint32_t int_key;
    string str_key;
    string name;
    Property prop;
    cmd = (PersCmd)isb.ReadInt8();
    if(cmd == PC_PING)
    {
		osb.WriteInt8(RESPONSE_OK);
        smsc_log_debug(log, "Ping received");
        return;
    }
    if(cmd != PC_BATCH)
    {
        pt = (ProfileType)isb.ReadInt8();
        if(pt != PT_ABONENT)
            int_key = isb.ReadInt32();
        else
            isb.ReadString(str_key);
    }
    switch(cmd)
    {
        case PC_DEL:
            isb.ReadString(name);
            DelCmdHandler(pt, int_key, str_key, name, osb);
            return;
        case PC_SET:
            prop.Deserialize(isb);
            SetCmdHandler(pt, int_key, str_key, prop, osb);
            return;
        case PC_GET:
            isb.ReadString(name);
            GetCmdHandler(pt, int_key, str_key, name, osb);
            return;
        case PC_INC:
            prop.Deserialize(isb);
            IncCmdHandler(pt, int_key, str_key, prop, osb);
            return;
        case PC_INC_MOD:
        {
            int inc = isb.ReadInt32();
            prop.Deserialize(isb);
            IncModCmdHandler(pt, int_key, str_key, prop, inc, osb);
			return;
        }
        case PC_BATCH:
        {
            uint16_t cnt = isb.ReadInt16();
            while(cnt--)
                    execCommand(isb, osb);
			return;
        }
        default:
            smsc_log_debug(log, "Bad command %d", cmd);
    }
}

}}
