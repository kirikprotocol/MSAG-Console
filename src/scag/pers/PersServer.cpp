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
    transactBatch = false;
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

PersServerResponseType PersServer::DelCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb)
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
    PersServerResponseType result = exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND;
    SendResponse(osb, result);
    return result;
}

PersServerResponseType PersServer::GetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, const std::string& name, SerialBuffer& osb)
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
        return RESPONSE_OK;
    }
    else
    {
        smsc_log_debug(plog, "GetCmdHandler property not found: store=%d, key=%s(%d), name=%s", pt, str_key.c_str(), int_key, name.c_str());    
        SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
        return RESPONSE_PROPERTY_NOT_FOUND;
    }        
}

PersServerResponseType PersServer::SetCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    PersServerResponseType response = RESPONSE_OK;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "SetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.toString().c_str());
        if (!AbonentStore->setProperty(str_key.c_str(), prop)) {
          response = RESPONSE_ERROR;
        }
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(plog, "SetCmdHandler store=%d, key=%d, prop=%s", pt, int_key, prop.toString().c_str());
        if (!is->setProperty(int_key, prop)) {
          response = RESPONSE_ERROR;
        }
    }
    SendResponse(osb, response);
    return response;
}

PersServerResponseType PersServer::IncCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    PersServerResponseType response = RESPONSE_OK;
    int result = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName());
        response = AbonentStore->incProperty(str_key.c_str(), prop, result);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(plog, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName());
        response = is->incProperty(int_key, prop, result);
    }
    SendResponse(osb, response);
    return response;
}

PersServerResponseType PersServer::IncResultCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, SerialBuffer& osb)
{
    IntProfileStore *is;
    PersServerResponseType response = RESPONSE_OK;
    int result = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "IncResultCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName());
        response = AbonentStore->incProperty(str_key.c_str(), prop, result);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(plog, "IncResultCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName());
        response = is->incProperty(int_key, prop, result);
    }
    SendResponse(osb, response);
    if (response == RESPONSE_OK) {
      osb.WriteInt32(result);
    }
    return response;
}

PersServerResponseType PersServer::IncModCmdHandler(ProfileType pt, uint32_t int_key, const std::string& str_key, Property& prop, int mod, SerialBuffer& osb)
{
    IntProfileStore *is;
    PersServerResponseType response = RESPONSE_OK;
    int res = 0;
    if(pt == PT_ABONENT)
    {
        smsc_log_debug(plog, "IncModCmdHandler AbonentStore: key=%s, name=%s, mod=%d", str_key.c_str(), prop.getName(), mod);
        response = AbonentStore->incModProperty(str_key.c_str(), prop, mod, res);
    }
    else if(is = findStore(pt))    
    {
        smsc_log_debug(plog, "IncModCmdHandler store=%d, key=%d, name=%s, mod=%d", pt, int_key, prop.getName(), mod);
        response = is->incModProperty(int_key, prop, mod, res);
    }
    SendResponse(osb, response);
    if (response == RESPONSE_OK) {
      osb.WriteInt32(res);
    }
    return response;
}
  
bool PersServer::processPacket(ConnectionContext& ctx)
{
    SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
    osb.SetPos(4);
    PersServerResponseType response = RESPONSE_ERROR;
    try {
      execCommand(isb, osb);
      SetPacketSize(osb);
      return true;
    }
    catch(const SerialBufferOutOfBounds &e) {
      smsc_log_warn(plog, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s",
                     isb.length(), isb.toString().c_str());
      response = RESPONSE_BAD_REQUEST;
    }
    catch(const std::runtime_error& e) {
      smsc_log_warn(plog, "std::runtime_error: Error profile key: %s. received buffer len=%d, data=%s",
                     e.what(), isb.length(), isb.toString().c_str());
      response = RESPONSE_BAD_REQUEST;
    }
    catch(const FileException& e) {
      smsc_log_warn(plog, "FileException: '%s'. received buffer len=%d, data=%s",
                     e.what(), isb.length(), isb.toString().c_str());
    }
    catch(const std::exception& e) {
      smsc_log_warn(plog, "std::exception: %s. received buffer len=%d, data=%s",
                     e.what(), isb.length(), isb.toString().c_str());
    }
    catch(...) {
      smsc_log_warn(plog, "Unknown exception. received buffer len=%d, data=%s",
                     isb.length(), isb.toString().c_str());
    }
    if (transactBatch) {
      rollbackCommands(response);
    }
    SendResponse(osb, response);
	SetPacketSize(osb);
    return true;
}

PersServerResponseType PersServer::execCommand(SerialBuffer& isb, SerialBuffer& osb)
{
    PersCmd cmd;
    ProfileType pt;
    uint32_t int_key;
    string profKey;
    string name;
    Property prop;
    cmd = (PersCmd)isb.ReadInt8();
    if(cmd == PC_PING)
    {
		osb.WriteInt8(RESPONSE_OK);
        smsc_log_debug(plog, "Ping received");
        return RESPONSE_OK;
    }
    if(cmd != PC_BATCH && cmd != PC_TRANSACT_BATCH)
    {
        pt = (ProfileType)isb.ReadInt8();
        if(pt != PT_ABONENT) {
          int_key = isb.ReadInt32();
        }
        else {
          isb.ReadString(profKey);
          //isb.ReadString(str_key);
          //profKey = getProfileKey(str_key);
        }
    }
    switch(cmd)
    {
        case PC_DEL:
            isb.ReadString(name);
            return DelCmdHandler(pt, int_key, profKey, name, osb);
        case PC_SET:
            prop.Deserialize(isb);
            return SetCmdHandler(pt, int_key, profKey, prop, osb);
        case PC_GET:
            isb.ReadString(name);
            return GetCmdHandler(pt, int_key, profKey, name, osb);
        case PC_INC:
            prop.Deserialize(isb);
            return IncCmdHandler(pt, int_key, profKey, prop, osb);
        case PC_INC_RESULT:
            prop.Deserialize(isb);
            return IncResultCmdHandler(pt, int_key, profKey, prop, osb);
        case PC_INC_MOD:
        {
            int mod = isb.ReadInt32();
            prop.Deserialize(isb);
            return IncModCmdHandler(pt, int_key, profKey, prop, mod, osb);
        }
        case PC_BATCH:
        {
            uint16_t cnt = isb.ReadInt16();
            while(cnt--)
                    execCommand(isb, osb);
			return RESPONSE_OK;
        }
        case PC_TRANSACT_BATCH: 
        {
          setNeedBackup(true);
          uint16_t cnt = isb.ReadInt16();
          while(cnt--) {
            PersServerResponseType result = execCommand(isb, osb);
            if (result != RESPONSE_OK) {
              rollbackCommands(result);
              return result;
            }
          }
          resetStroragesBackup();
          return RESPONSE_OK;
        }
        default: {
          smsc_log_warn(plog, "Bad command %d", cmd);
          SendResponse(osb, RESPONSE_BAD_REQUEST);
          return RESPONSE_BAD_REQUEST;
        }
    }
}

Profile* PersServer::getProfile(const string& key) {
  try {
    AbntAddr addr(key.c_str());
    Profile *pf = AbonentStore->getProfile(addr, false); 
    //Profile *pf = AbonentStore->_getProfile(addr, false);
    return pf; 
  } catch (const std::exception& e) {
    smsc_log_warn(plog, "std::exception: %s", e.what());
    return NULL;
  }
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

void PersServer::rollbackCommands(PersServerResponseType error_code) {
  smsc_log_warn(plog, "Rollback batch commands. Error in transactional batch, error code=%d", error_code);
  AbonentStore->rollBack();
  for (int i = 0; i < INT_STORE_CNT; ++i) {
    int_store[i].store->rollBack();
  }
  transactBatch = false;
}

void PersServer::resetStroragesBackup() {
  smsc_log_debug(plog, "Reset strorages backup");
  AbonentStore->resetBackup();
  for (int i = 0; i < INT_STORE_CNT; ++i) {
    int_store[i].store->resetBackup();
  }
  transactBatch = false;
}

void PersServer::setNeedBackup(bool needBackup) {
  transactBatch = needBackup;
  AbonentStore->setNeedBackup(needBackup);
  for (int i = 0; i < INT_STORE_CNT; ++i) {
    int_store[i].store->setNeedBackup(needBackup);
  }
}

}}
