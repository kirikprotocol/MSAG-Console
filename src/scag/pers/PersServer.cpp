/* $Id$ */

#include "PersServer.h"
#include "sms/sms.h"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::logger::Logger;
using smsc::sms::Address;

PersServer::PersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_, int transactTimeout_,
                        StringProfileStore *abonent, IntProfileStore *service, IntProfileStore *oper, IntProfileStore *provider):
        PersSocketServer(persHost_, persPort_, maxClientCount_, timeout_, transactTimeout_), plog(Logger::getInstance("persserver"))
{
    int_store[0].pt = PT_SERVICE;
    int_store[1].pt = PT_OPERATOR;
    int_store[2].pt = PT_PROVIDER;
    int_store[0].store = service;
    int_store[1].store = oper;
    int_store[2].store = provider;
    AbonentStore = abonent;
}

IntProfileStore* PersServer::findStore(ProfileType pt)
{
    int i = 0;
    while(i < INT_STORE_CNT && pt != int_store[i].pt) i++;
    return i < INT_STORE_CNT ? int_store[i].store : NULL;            
}

void PersServer::SendResponse(SerialBuffer& sb, PersServerResponseType r)
{
    sb.WriteInt8((uint8_t)r);
}

void PersServer::SetPacketSize(SerialBuffer& sb)
{
    sb.SetPos(0);
    sb.WriteInt32(sb.GetSize());
}

void PersServer::DelCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb)
{
    IntProfileStore *is;
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "DelCmdHandler AbonetStore: key=%s, name=%s", str_key.c_str(), name.c_str());
        exists = AbonentStore->delProperty(str_key.c_str(), name.c_str());
    }
    else if(is = findStore(pt))
    {
        smsc_log_debug(plog, "DelCmdHandler store= %d, key=%d, name=%s", pt, int_key, name.c_str());
        exists = is->delProperty(int_key, name.c_str());
    }
    SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void PersServer::GetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb)
{
    IntProfileStore *is;
    Property prop;
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "GetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), name.c_str());
        exists = AbonentStore->getProperty(str_key.c_str(), name.c_str(), prop);
    }
    else if(is = findStore(pt))
    {
        smsc_log_debug(plog, "GetCmdHandler store=%d, key=%d, name=%s", pt, int_key, name.c_str());
        exists = is->getProperty(int_key, name.c_str(), prop);
    }
    if(exists)
    {   
        smsc_log_debug(plog, "GetCmdHandler prop=%s", prop.toString().c_str());
        SendResponse(osb, RESPONSE_OK);
        prop.Serialize(osb);
    }
    else
    {
        smsc_log_debug(plog, "GetCmdHandler property not found: store=%d, key=%s(%d), name=%s", pt, str_key.c_str(), int_key, name.c_str());    
        SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
    }        
}

void PersServer::SetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "SetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.toString().c_str());
        AbonentStore->setProperty(str_key.c_str(), prop);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(plog, "SetCmdHandler store=%d, key=%d, prop=%s", pt, int_key, prop.toString().c_str());
        is->setProperty(int_key, prop);
    }
    SendResponse(osb, RESPONSE_OK);
}

void PersServer::IncCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    bool exists = false;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName().c_str());
        exists = AbonentStore->incProperty(str_key.c_str(), prop);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(plog, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName().c_str());
        exists = is->incProperty(int_key, prop);
    }
    SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void PersServer::IncModCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, int mod, SerialBuffer& osb)
{
    IntProfileStore *is;
    bool exists = false;
    int res = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "IncModCmdHandler AbonentStore: key=%s, name=%s, mod=%d", str_key.c_str(), prop.getName().c_str(), mod);
        exists = AbonentStore->incModProperty(str_key.c_str(), prop, mod, res);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(plog, "IncModCmdHandler store=%d, key=%d, name=%s, mod=%d", pt, int_key, prop.getName().c_str(), mod);
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

bool PersServer::processPacket(ConnectionContext& ctx)
{
    SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
    osb.SetPos(4);
    try{
		execCommand(isb, osb);
		SetPacketSize(osb);
		return true;
    }
    catch(const SerialBufferOutOfBounds &e)
    {
        smsc_log_debug(plog, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s", isb.length(), isb.toString().c_str());
    }
    catch(const std::runtime_error& e) {
      smsc_log_debug(plog, "Error profile key: %s", e.what());
    }
    catch(const Exception& e)
    {
        smsc_log_debug(plog, "Exception: \'%s\'. Bad data in buffer received len=%d, data=%s", e.what(), isb.length(), isb.toString().c_str());
    }
    catch(...)
    {
        smsc_log_debug(plog, "Bad data in buffer received len=%d, data=%s", isb.length(), isb.toString().c_str());
    }
    SendResponse(osb, RESPONSE_BAD_REQUEST);
	SetPacketSize(osb);
    return true;
}

void PersServer::execCommand(SerialBuffer& isb, SerialBuffer& osb)
{
    PersCmd cmd;
    ProfileType pt;
    uint32_t int_key;
    string str_key;
    string profKey;
    string name;
    Property prop;
    cmd = (PersCmd)isb.ReadInt8();
    if(cmd == PC_PING)
    {
		osb.WriteInt8(RESPONSE_OK);
        smsc_log_debug(plog, "Ping received");
        return;
    }
    if(cmd != PC_BATCH)
    {
        pt = (ProfileType)isb.ReadInt8();
        if(pt != PT_ABONENT) {
          int_key = isb.ReadInt32();
        }
        else {
          isb.ReadString(str_key);
          profKey = getProfileKey(str_key);
        }
    }
    switch(cmd)
    {
        case PC_DEL:
            isb.ReadString(name);
            DelCmdHandler(pt, int_key, profKey, name, osb);
            return;
        case PC_SET:
            prop.Deserialize(isb);
            SetCmdHandler(pt, int_key, profKey, prop, osb);
            return;
        case PC_GET:
            isb.ReadString(name);
            GetCmdHandler(pt, int_key, profKey, name, osb);
            return;
        case PC_INC:
            prop.Deserialize(isb);
            IncCmdHandler(pt, int_key, profKey, prop, osb);
            return;
        case PC_INC_MOD:
        {
            int inc = isb.ReadInt32();
            prop.Deserialize(isb);
            IncModCmdHandler(pt, int_key, profKey, prop, inc, osb);
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
            smsc_log_debug(plog, "Bad command %d", cmd);
    }
}

Profile* PersServer::getProfile(const string& key) {
  AbntAddr addr(key.c_str());
  Profile *pf = AbonentStore->getProfile(addr, false); 
  //Profile *pf = AbonentStore->_getProfile(addr, false);
  return pf; 
}

Profile* PersServer::createProfile(AbntAddr& addr) {
  return  AbonentStore->createProfile(addr);
}

string PersServer::getProfileKey(const string& key) const {
  Address addr(key.c_str());
  if (key[0] != '.') {
    addr.setNumberingPlan(1);
    addr.setTypeOfNumber(1);
  }
  return addr.toString();
}


}}
