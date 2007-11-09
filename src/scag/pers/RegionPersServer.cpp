#include "RegionPersServer.h"
#include "util/Exception.hpp"
#include "Types.h"

namespace scag { namespace pers {

using smsc::util::Exception;

RegionPersServer::RegionPersServer(const char* persHost_, int persPort_, int maxClientCount_, 
                   int timeout_, StringProfileStore *abonent, IntProfileStore *service,
                   IntProfileStore *oper, IntProfileStore *provider, const string& _central_host,
                   int _central_port, uint32_t _region_id, const string& _region_psw):

                  PersServer(persHost_, persPort_, maxClientCount_, timeout_, abonent, service,
                  oper, provider), central_host(_central_host), central_port(_central_port),
                  region_id(_region_id), region_psw(_region_psw), connected(false),
                  central_socket(0) {
  rplog = Logger::getInstance("rpserver");
}


void RegionPersServer::connectToCP() {
  smsc_log_info(rplog, "Connecting to central persserver host=%s:%d timeout=%d",
                 central_host.c_str(), central_port, timeout);
  central_socket = new Socket();
  if (central_socket->Init(central_host.c_str(), central_port, timeout) == -1
      || central_socket->Connect() == -1) {
    throw Exception("connectToCentralPers: can't connect to central perserver host %s:%d",
                    central_host.c_str(), central_port);
  }

  char resp[3];
  if (central_socket->Read(resp, 2) != 2) {
    throw Exception("connectToCentralPers: can't connect to central perserver host %s:%d",
                    central_host.c_str(), central_port);
  }
  resp[2] = 0;
  if (!strcmp(resp, "SB")) {
    throw Exception("connectToCentralPers: central perserver busy");
  } 
  if (strcmp(resp, "OK")) {
    throw Exception("connectToCentralPers: unknown response");
  }
  LoginCmd login_cmd(region_id, region_psw);
  SerialBuffer sb;
  login_cmd.serialize(sb);
  int login_length = sb.length();
  int n = central_socket->WriteAll(sb.c_ptr(), login_length);
  if (n < login_length) {
    throw Exception("connectToCentralPers: can't bind to central perserver");
  }
}

bool RegionPersServer::bindToCP() {
  if (connected) {
    return true;
  }
  try {
    connectToCP();
    ConnectionContext *cxt = new ConnectionContext();
    central_socket->setNonBlocking(1);
    central_socket->setData(0, cxt);
    listener.addRW(central_socket);
    connected = true;
    ++clientCount;
    smsc_log_debug(rplog, "bindToCP : region perserver bound to CP socket:%p cxt:%p",
                    central_socket, cxt);
    return true;
  } catch (const Exception& e) {
    smsc_log_error(rplog, "%s", e.what());
    central_socket->Close();
    delete central_socket;
    return false;
  }
}

bool RegionPersServer::processPacket(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "processPacket");
  ConnectionContext *central_ctx = (ConnectionContext *)central_socket->getData(0);
  if (!central_ctx) {
    connected = false;
    smsc_log_debug(rplog, "processPacket : central_socket hasn't ctx");
    return false;
  }
  return central_ctx == &ctx ? processPacketFromCP(ctx) : PersServer::processPacket(ctx);
}

void RegionPersServer::execCommand(SerialBuffer& isb, SerialBuffer& osb) {
  smsc_log_debug(rplog, "execCommand(isb, osb)");
  PersCmd cmd;
  ProfileType pt;
  uint32_t int_key;
  string str_key;
  //isb.setPos(4);
  cmd = (PersCmd)isb.ReadInt8();
  if(cmd == PC_PING) {
    osb.WriteInt8(RESPONSE_OK);
    smsc_log_debug(rplog, "Ping received");
    return;
  }
  if (cmd == PC_BATCH) {
    uint16_t cnt = isb.ReadInt16();
    while(cnt--) {
      execCommand(isb, osb);
    }
    return;
  }
  pt = (ProfileType)isb.ReadInt8();
  smsc_log_debug(rplog, "profile type = %d", pt);
  if(pt != PT_ABONENT) {
    int_key = isb.ReadInt32();
    string name;
    Property prop;
    switch(cmd) {
    case PC_DEL:
      isb.ReadString(name);
      DelCmdHandler(pt, int_key, name, osb);
      return;
    case PC_SET:
      prop.Deserialize(isb);
      SetCmdHandler(pt, int_key, prop, osb);
      return;
    case PC_GET:
      isb.ReadString(name);
      GetCmdHandler(pt, int_key, name, osb);
      return;
    case PC_INC:
      prop.Deserialize(isb);
      IncCmdHandler(pt, int_key, prop, osb);
      return;
    case PC_INC_MOD:
    {
      int inc = isb.ReadInt32();
      prop.Deserialize(isb);
      IncModCmdHandler(pt, int_key, prop, inc, osb);
      return;
    }
    default:
      smsc_log_debug(rplog, "Bad command %d", cmd);
    }
    return;
  } 
  isb.ReadString(str_key);
  smsc_log_debug(rplog, "profile key = \'%s\'", str_key.c_str());
  AbntAddr addr(str_key.c_str());
  uint8_t pf_state = OK;
  profiles_states.Get(addr, pf_state);
  if (pf_state != OK) {
    smsc_log_debug(rplog, "execCommand(isb, osb) : profile key=%s LOCKED or DELETED",
                    str_key.c_str());
    SendResponse(osb, RESPONSE_ERROR);
    return;
  }
  Profile *pf = getProfile(str_key);
  if (pf) {
    smsc_log_debug(rplog, "execCommand(isb, osb) : profile key=%s found int local storage", 
                   str_key.c_str());
    execCommand(cmd, pf, str_key, isb, osb);
    return;
  }
  smsc_log_debug(rplog, "execCommand(isb, osb) : profile key=%s NOT found int local storage",
                  str_key.c_str());
  GetProfileCmd get_profile_cmd(str_key);
  if (!sendCommandToCP(get_profile_cmd)) {
    smsc_log_warn(rplog, "execCommand(isb, osb) : can't send command to CP");
    SendResponse(osb, RESPONSE_ERROR);
  }
  commands.Insert(AbntAddr(str_key.c_str()), CmdContext(cmd, isb, &osb));
}

void RegionPersServer::execCommand(PersCmd cmd, Profile *pf, const string& str_key,
                                    SerialBuffer& isb, SerialBuffer& osb){
  smsc_log_debug(rplog, "execCommand");
  string name;
  Property prop;
  switch(cmd) {
  case PC_DEL:
    isb.ReadString(name);
    DelCmdHandler(pf, str_key, name, osb);
    return;
  case PC_SET:
    prop.Deserialize(isb);
    SetCmdHandler(pf, str_key, prop, osb);
    return;
  case PC_GET:
    isb.ReadString(name);
    GetCmdHandler(pf, str_key, name, osb);
    return;
  case PC_INC:
    prop.Deserialize(isb);
    IncCmdHandler(pf, str_key, prop, osb);
    return;
  case PC_INC_MOD:
  {
    int inc = isb.ReadInt32();
    prop.Deserialize(isb);
    IncModCmdHandler(pf, str_key, prop, inc, osb);
    return;
  }
  default:
    smsc_log_debug(rplog, "Bad command %d", cmd);
  }
}

bool RegionPersServer::processPacketFromCP(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "processPacketFromCP : process packet %p", &ctx);
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;

  try {
    uint8_t cmd = isb.ReadInt8();
    smsc_log_debug(rplog, "cmd=%d", cmd);

    switch(cmd) {

    case CentralPersCmd::GET_PROFILE:
      getProfileCmdHandler(ctx);
      smsc_log_debug(rplog, "GET_PROFILE Profile received");
      break;

    case CentralPersCmd::PROFILE_RESP:
      profileRespCmdHandler(ctx);
      smsc_log_debug(rplog, "PROFILE Resp received");
      break;

    case CentralPersCmd::DONE_RESP:
      doneRespCmdHandler(ctx);
      smsc_log_debug(rplog, "DONE_RESP received");
      break;

    case CentralPersCmd::DONE:
      doneCmdHandler(ctx);
      smsc_log_debug(rplog, "DONE received");
      break;

    case CentralPersCmd::CHECK_OWN:
      checkOwnCmdHandler(ctx);
      smsc_log_debug(rplog, "CHECK_OWN received");
      break;
    }
  } catch (const SerialBufferOutOfBounds& e) {
    smsc_log_warn(rplog, "processPacketFromCP : SerialBufferOutOfBounds exception");
  }
  return true;
}

void RegionPersServer::onDisconnect(ConnectionContext &ctx) {
  ConnectionContext *central_ctx = (ConnectionContext *)central_socket->getData(0);
  if (!central_ctx) {
    connected = false;
    smsc_log_debug(rplog, "onDisconnect : central_socket hasn't ctx");
  }
  if (central_ctx == &ctx) {
    connected = false;
    smsc_log_debug(rplog, "onDisconnect : central_socket disconnected");
    //сохранить контекст чтоб потом его продолжить обрабатывать после переподключения
  }
}

bool RegionPersServer::sendCommandToCP(const CPersCmd& cmd) {
  smsc_log_debug(rplog, "sendCommandToCP: cmd_id=%d", cmd.cmd_id);
  if (!bindToCP()) {
    return false;
  }
  ConnectionContext *ctx = (ConnectionContext*)central_socket->getData(0);
  if (!ctx) {
    return false;
  }
  cmd.serialize(ctx->outbuf);
  return true;
}

void RegionPersServer::getProfileCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "getProfileCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  GetProfileCmd get_profile_cmd;
  get_profile_cmd.deserialize(isb);
  Profile *pf = getProfile(get_profile_cmd.key);
  ProfileRespCmd profile_resp(get_profile_cmd.key);
  if (!pf) {
    // profile not found
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler : profile with key=%s not found", get_profile_cmd.key.c_str());
    //return;
  }
  AbntAddr addr(get_profile_cmd.key.c_str());
  uint8_t pf_state = OK;
  profiles_states.Get(addr, pf_state);
  //if (pf->getState() == LOCKED) {
  switch (pf_state) {
  case LOCKED : 
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler : profile with key=%s LOCKED", get_profile_cmd.key.c_str());
    break;
  case DELETED :
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler : profile with key=%s DELETED", get_profile_cmd.key.c_str());
    break;
  case OK : 
    //pf->setLocked();
    profiles_states.Insert(addr, LOCKED);
    smsc_log_warn(rplog, "getProfileCmdHandler : set profile state LOCKED key=%s", get_profile_cmd.key.c_str());
    profile_resp.profile = pf;
    break;
  default:
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler : profile with key=%s has unknown state", get_profile_cmd.key.c_str());
  }
  sendCommandToCP(profile_resp);
}

void RegionPersServer::profileRespCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "profileRespCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  ProfileRespCmd profile_resp;
  profile_resp.deserialize(isb);
  CmdContext cmd_ctx;
  AbntAddr addr(profile_resp.key.c_str());
  if (!(commands.Get(addr, cmd_ctx))) {
    smsc_log_error(rplog, "profileRespCmdHandler : command conntext with key=%s not found",
                    profile_resp.key.c_str());
    return;
  }
  if (!profile_resp.is_ok) {
    smsc_log_error(rplog, "profileRespCmdHandler : receive error ProfileResp with key=%s",
                   profile_resp.key.c_str());
    if (cmd_ctx.osb) {
      SendResponse(*cmd_ctx.osb, RESPONSE_ERROR);
    } else {
      smsc_log_error(rplog, "profileRespCmdHandler : client connection error key=%s ctx=%p",
                      profile_resp.key.c_str(), &ctx);
    }
    commands.Delete(addr);
    return;
  }
  StringProfileStore* store = getAbonentStore();
  AbntAddr store_addr(profile_resp.key.c_str());
  //Profile* new_profile = store->createProfile(store_addr);
  std::auto_ptr<Profile> profile(store->createProfile(store_addr));
  if (profile_resp.profile) {
    profile_resp.profile->copyPropertiesTo(profile.get());
    //profile.get()->setLocked();
    getAbonentStore()->storeProfile(store_addr, profile.get());
    profiles_states.Insert(addr, LOCKED);
    DoneCmd done(1);
    sendCommandToCP(done);
    return;
  }
  try {
    if (cmd_ctx.osb) {
      execCommand(cmd_ctx.cmd_id, profile.get(), profile_resp.key, cmd_ctx.isb, *cmd_ctx.osb);
    } else {
      smsc_log_error(rplog, "profileRespCmdHandler : client connection error key=%s ctx=%p",
                      profile_resp.key.c_str(), &ctx);
    }
  } catch (const SerialBufferOutOfBounds& e) {
    smsc_log_warn(rplog, "profileRespCmdHandler : SerialBufferOutOfBounds exception");
  }
  commands.Delete(addr);
}

void RegionPersServer::doneCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "doneCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  DoneCmd done;
  done.deserialize(isb);
  AbntAddr addr(done.key.c_str());
  profiles_states.Delete(addr);
  if (done.is_ok) {
    getAbonentStore()->deleteProfile(addr);
    DoneRespCmd done_resp(1);
    sendCommandToCP(done_resp);
  } else {
    //что делать если пришёл DONE=ERROR
  }
}

void RegionPersServer::doneRespCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "doneRespCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  DoneCmd done_resp;
  done_resp.deserialize(isb);
  AbntAddr addr(done_resp.key.c_str());
  if (done_resp.is_ok) {
    //profile_states.In
    profiles_states.Insert(addr, OK);
    CmdContext cmd_ctx;
    if (!(commands.Get(addr, cmd_ctx))) {
      smsc_log_error(rplog, "doneRespCmdHandler : command conntext with key=%s not found ctx=%p",
                      done_resp.key.c_str(), &ctx);
      return;
    }
    if (!cmd_ctx.osb) {
      smsc_log_error(rplog, "doneRespCmdHandler : client connection error key=%s ctx=%p",
                      done_resp.key.c_str(), &ctx);
      return;
    }
    Profile *pf = getProfile(done_resp.key);
    try {
      if (pf) {
        execCommand(cmd_ctx.cmd_id, pf, done_resp.key, cmd_ctx.isb, *cmd_ctx.osb);
        return;
      } 
      smsc_log_debug(rplog, "doneRespCmdHandler : profile with key=%s not found ctx=%p",
                      done_resp.key.c_str(), &ctx);
    } catch (const SerialBufferOutOfBounds& e) {
      smsc_log_warn(rplog, "profileRespCmdHandler : SerialBufferOutOfBounds exception");
    }
    SendResponse(*cmd_ctx.osb, RESPONSE_ERROR);
  } else {
    //что делать если пришёл DONE_RESP=ERROR
  }
}

void RegionPersServer::checkOwnCmdHandler(ConnectionContext &ctx) {
}

void RegionPersServer::DelCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb) {
  smsc_log_debug(rplog, "DelCmdHandler");
  IntProfileStore *is;
  bool exists = false;
  if(is = findStore(pt)) {
    smsc_log_debug(rplog, "DelCmdHandler store= %d, key=%d, name=%s", pt, int_key, name.c_str());
    exists = is->delProperty(int_key, name.c_str());
  }
  SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void RegionPersServer::GetCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb) {
  smsc_log_debug(rplog, "GetCmdHandler");
  IntProfileStore *is;
  Property prop;
  bool exists = false;
  if(is = findStore(pt)) {
    smsc_log_debug(rplog, "GetCmdHandler store=%d, key=%d, name=%s", pt, int_key, name.c_str());
    exists = is->getProperty(int_key, name.c_str(), prop);
  }
  if(exists) {   
    smsc_log_debug(rplog, "GetCmdHandler prop=%s", prop.toString().c_str());
    SendResponse(osb, RESPONSE_OK);
    prop.Serialize(osb);
  } else {
    smsc_log_debug(rplog, "GetCmdHandler property not found: store=%d, key=%d, name=%s", pt, int_key, name.c_str());    
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
  }        
}

void RegionPersServer::SetCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb) {
  smsc_log_debug(rplog, "SetCmdHandler");
  IntProfileStore *is;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "SetCmdHandler store=%d, key=%d, prop=%s", pt, int_key, prop.toString().c_str());
    is->setProperty(int_key, prop);
  }
  SendResponse(osb, RESPONSE_OK);
}

void RegionPersServer::IncCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb) {
  smsc_log_debug(rplog, "IncCmdHandler");
  IntProfileStore *is;
  bool exists = false;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName().c_str());
    exists = is->incProperty(int_key, prop);
  }
  SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void RegionPersServer::IncModCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, int mod, SerialBuffer& osb) {
  smsc_log_debug(rplog, "IncModCmdHandler");
  IntProfileStore *is;
  bool exists = false;
  int res = 0;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "IncModCmdHandler store=%d, key=%d, name=%s, mod=%d", pt, int_key, prop.getName().c_str(), mod);
    exists = is->incModProperty(int_key, prop, mod, res);
  }
  if(exists) {
    SendResponse(osb, RESPONSE_OK);
    osb.WriteInt32(res);
  } else {
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
  }
}

void RegionPersServer::DelCmdHandler(Profile *pf, const string& str_key,
                                      const string& name, SerialBuffer& osb) {
  smsc_log_debug(rplog, "DelCmdHandler AbonetStore: key=%s, name=%s", str_key.c_str(), name.c_str());
  bool exists = getAbonentStore()->delProperty(str_key.c_str(), name.c_str());
  SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void RegionPersServer::GetCmdHandler(Profile *pf, const string& str_key,
                                const string& name, SerialBuffer& osb) {
  Property prop;
  smsc_log_debug(rplog, "GetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), name.c_str());
  if(getAbonentStore()->getProperty(str_key.c_str(), name.c_str(), prop)) {   
    smsc_log_debug(rplog, "GetCmdHandler prop=%s", prop.toString().c_str());
    SendResponse(osb, RESPONSE_OK);
    prop.Serialize(osb);
  } else {
    smsc_log_debug(rplog, "GetCmdHandler property not found: AbonentStore key=%s, name=%s", str_key.c_str(), name.c_str());    
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
  }        
}

void RegionPersServer::SetCmdHandler(Profile *pf, const string& str_key,
                                Property& prop, SerialBuffer& osb) {
  smsc_log_debug(rplog, "SetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.toString().c_str());
  getAbonentStore()->setProperty(str_key.c_str(), prop);
  SendResponse(osb, RESPONSE_OK);
}

void RegionPersServer::IncCmdHandler(Profile *pf, const string& str_key,
                                Property& prop, SerialBuffer& osb) {
  smsc_log_debug(rplog, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName().c_str());
  bool exists = getAbonentStore()->incProperty(str_key.c_str(), prop);
  SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void RegionPersServer::IncModCmdHandler(Profile *pf, const string& str_key,
                                         Property& prop, int mod, SerialBuffer& osb) {
  int res = 0;
  smsc_log_debug(rplog, "IncModCmdHandler AbonentStore: key=%s, name=%s, mod=%d", str_key.c_str(), prop.getName().c_str(), mod);
  if(getAbonentStore()->incModProperty(str_key.c_str(), prop, mod, res)) {
    SendResponse(osb, RESPONSE_OK);
    osb.WriteInt32(res);
  } else {
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
  }
}


/*
void RegionPersServer::setProperty(Profile* pf, Property& prop) {
  if (!pf) {
    return;
  }
  MutexGuard mt(store_mtx);
  if(prop.isExpired()) {
    return;
  }
  Property* p = pf->GetProperty(prop.getName().c_str());
  if(p != NULL) {
    p->setValue(prop);
    p->WriteAccess();
  } else {
    pf->AddProperty(prop);
  }
  //Abonen storeProfile(key, pf);
  //smsc_log_info(dblog, "%c key=\"%s\" property=%s", p ? 'U' : 'A', key.toString().c_str(), p ? p->toString().c_str() : prop.toString().c_str());
}

bool RegionPersServer::delProperty(Profile* pf, const char* nm) {
}

bool RegionPersServer::getProperty(Profile* pf, const char* nm, Property& prop) {
}

bool RegionPersServer::incProperty(Profile* pf, Property& prop) {
}

bool RegionPersServer::incModProperty(Profile* pf, Property& prop, uint32_t mod, int& res) {
}
*/
CmdContext::CmdContext():cmd_id(PC_UNKNOWN), osb(0) {};

CmdContext::CmdContext(PersCmd _cmd_id, SerialBuffer& _isb, SerialBuffer* _osb) :
             cmd_id(_cmd_id), osb(_osb) {
  isb.blkwrite(_isb.c_curPtr(), _isb.length() - _isb.getPos());
  isb.setPos(0);
}

CmdContext::CmdContext(const CmdContext& ctx):cmd_id(ctx.cmd_id), osb(ctx.osb) {
  isb.blkwrite(ctx.isb.c_curPtr(), ctx.isb.length() - ctx.isb.getPos());
  isb.setPos(0);
}

CmdContext& CmdContext::operator=(const CmdContext& ctx) {
  if (this == &ctx) {
    return *this;
  }
  isb.Empty();
  isb.blkwrite(ctx.isb.c_curPtr(), ctx.isb.length() - ctx.isb.getPos());
  isb.setPos(0);
  cmd_id = ctx.cmd_id;
  osb = ctx.osb;
  return *this;
}


}//pers
}//scag

