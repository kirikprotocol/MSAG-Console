#include "RegionPersServer.h"
#include "util/Exception.hpp"
#include "Types.h"
#include "sms/sms.h"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::sms::Address;

RegionPersServer::RegionPersServer(const char* persHost_, int persPort_, int maxClientCount_, 
                   int timeout_, int transactTimeout_, StringProfileStore *abonent, IntProfileStore *service,
                   IntProfileStore *oper, IntProfileStore *provider, const string& _central_host,
                   int _central_port, uint32_t _region_id, const string& _region_psw):

                  PersServer(persHost_, persPort_, maxClientCount_, timeout_, transactTimeout_,
                  abonent, service, oper, provider),
                  central_host(_central_host), central_port(_central_port),
                  region_id(_region_id), region_psw(_region_psw), connected(false),
                  central_socket(0), last_check_time(time(NULL)) {
  rplog = Logger::getInstance("rpserver");
  smsc_log_info(rplog, "transactTimeout = %d", transactTimeout);

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
    ++clientCount;
    connected = true;
    ConnectionContext *ctx = new ConnectionContext(central_socket);
    central_socket->setNonBlocking(1);
    central_socket->setData(0, ctx);
    //listener.addRW(central_socket);
    ctx->outbuf.SetPos(PACKET_LENGTH_SIZE);
    listener.addR(central_socket);
    smsc_log_debug(rplog, "bindToCP: region perserver bound to CP socket:%p cxt:%p",
                    central_socket, ctx);
    return true;
  } catch (const Exception& e) {
    smsc_log_error(rplog, "%s", e.what());
    central_socket->Close();
    delete central_socket;
    central_socket = NULL;
    connected = false;
    return false;
  }
}

bool RegionPersServer::processPacket(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "processPacket");
  if (!connected) {
    return processPacketFromClient(ctx);
  }
  ConnectionContext *central_ctx = (ConnectionContext *)central_socket->getData(0);
  if (!central_ctx) {
    connected = false;
    removeSocket(central_socket);
    smsc_log_debug(rplog, "processPacket: central_socket hasn't ctx");
    //return false;
    return processPacketFromClient(ctx);
    //return PersServer::processPacket(ctx);
  }
  if (central_ctx == &ctx) {
    try {
      smsc_log_debug(rplog, "Start process packet from CP");
      ctx.inbuf.setPos(PACKET_LENGTH_SIZE);
      processPacketFromCP(ctx);
      smsc_log_debug(rplog, "End process packet from CP");
    } catch(const SerialBufferOutOfBounds &e) {
      smsc_log_debug(rplog, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s",
                      ctx.inbuf.length(), ctx.inbuf.toString().c_str());
      ctx.inbuf.Empty();
    }
    catch(const Exception& e) {
      smsc_log_debug(rplog, "Exception: \'%s\'. Bad data in buffer received len=%d, data=%s",
                      e.what(), ctx.inbuf.length(), ctx.inbuf.toString().c_str());
    }
    catch(const std::runtime_error& e) {
      smsc_log_debug(rplog, "Error profile key: %s", e.what());
    }
    catch(...) {
      smsc_log_debug(rplog, "Some Exception. Bad data in buffer received len=%d, data=%s",
                      ctx.inbuf.length(), ctx.inbuf.toString().c_str());
    }
    return true;
  } else {
    return processPacketFromClient(ctx);
  }
}

bool RegionPersServer::processPacketFromCP(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "processPacketFromCP : process packet %p", &ctx);
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;

    uint8_t cmd = isb.ReadInt8();
    smsc_log_debug(rplog, "cmd=%d", cmd);

    switch(cmd) {

    case CentralPersCmd::GET_PROFILE:
      smsc_log_debug(rplog, "GET_PROFILE received");
      getProfileCmdHandler(ctx);
      break;

    case CentralPersCmd::PROFILE_RESP:
      smsc_log_debug(rplog, "PROFILE_RESP received");
      profileRespCmdHandler(ctx);
      break;

    case CentralPersCmd::DONE_RESP:
      smsc_log_debug(rplog, "DONE_RESP received");
      doneRespCmdHandler(ctx);
      break;

    case CentralPersCmd::DONE:
      smsc_log_debug(rplog, "DONE received");
      doneCmdHandler(ctx);
      break;

    case CentralPersCmd::CHECK_OWN:
      smsc_log_debug(rplog, "CHECK_OWN received");
      checkOwnCmdHandler(ctx);
      break;
    }

  if (ctx.inbuf.getPos() < ctx.inbuf.GetSize()) {
    smsc_log_debug(rplog, "processPacketFromCP: continue processing packet inbuf pos=%d length=%d",
                   isb.getPos(), isb.GetSize());
    processPacketFromCP(ctx);
  }
  return true;
}

bool RegionPersServer::processPacketFromClient(ConnectionContext& ctx) {
  smsc_log_debug(rplog, "processPacketFromClient");
  SerialBuffer &isb = ctx.inbuf;
  try {
    ctx.outbuf.SetPos(PACKET_LENGTH_SIZE);
    execCommand(ctx);
    if (ctx.outbuf.GetPos() > PACKET_LENGTH_SIZE) {
      SetPacketSize(ctx.outbuf);
    }
    return true;
  } catch(const SerialBufferOutOfBounds &e) {
    smsc_log_debug(rplog, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s", isb.length(), isb.toString().c_str());
  }
  catch(const std::runtime_error& e) {
    smsc_log_debug(rplog, "Error profile key: %s", e.what());
  }
  catch(const Exception& e) {
    smsc_log_debug(rplog, "Exception: \'%s\'. Bad data in buffer received len=%d, data=%s", e.what(), isb.length(), isb.toString().c_str());
  }
  catch(...) {
    smsc_log_debug(rplog, "Bad data in buffer received len=%d, data=%s", isb.length(), isb.toString().c_str());
  }
  createResponseForClient(&ctx.outbuf, RESPONSE_BAD_REQUEST);
  SetPacketSize(ctx.outbuf);
  return true;
}

void RegionPersServer::execCommand(ConnectionContext& ctx) {
  smsc_log_debug(rplog, "execCommand(ctx)");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  PersCmd cmd;
  ProfileType pt;
  uint32_t int_key;
  string str_key;
  cmd = (PersCmd)isb.ReadInt8();
  if(cmd == PC_PING) {
    smsc_log_debug(rplog, "execCommand(ctx): Ping received");
    createResponseForClient(&osb, RESPONSE_OK);
    return;
  }
  if (cmd == PC_BATCH) {
    uint16_t cnt = isb.ReadInt16();
    ctx.batch = true;
    ctx.batch_cmd_count = cnt;
    smsc_log_debug(rplog, "execCommand(ctx): Batch start. Commands count=%d", cnt);
    while(cnt--) {
      execCommand(ctx);
    }
    smsc_log_debug(rplog, "execCommand(ctx): Batch end");
    return;
  }
  if (ctx.batch && ctx.batch_cmd_count) {
    --ctx.batch_cmd_count;
  }
  pt = (ProfileType)isb.ReadInt8();
  smsc_log_debug(rplog, "execCommand(ctx): profile type = %d", pt);
  if(pt != PT_ABONENT) {
    int_key = isb.ReadInt32();
    string name;
    Property prop;
    //osb.SetPos(4);
    switch(cmd) {
    case PC_DEL:
      isb.ReadString(name);
      DelCmdHandler(pt, int_key, name, osb);
      break;
    case PC_SET:
      prop.Deserialize(isb);
      SetCmdHandler(pt, int_key, prop, osb);
      break;
    case PC_GET:
      isb.ReadString(name);
      GetCmdHandler(pt, int_key, name, osb);
      break;
    case PC_INC:
      prop.Deserialize(isb);
      IncCmdHandler(pt, int_key, prop, osb);
      break;
    case PC_INC_MOD:
    {
      int inc = isb.ReadInt32();
      prop.Deserialize(isb);
      IncModCmdHandler(pt, int_key, prop, inc, osb);
      break;
    }
    default:
      smsc_log_debug(rplog, "execCommand(ctx): Bad command %d", cmd);
    }
    //SetPacketSize(osb);
    return;
  } 
  isb.ReadString(str_key);
  string profKey = getProfileKey(str_key);
  smsc_log_debug(rplog, "execCommand(ctx): profile key=%s", profKey.c_str());
//
  Profile *pf = getProfile(profKey);
  if (!pf || pf->getState() == DELETED) {
    string err_str = !pf ? "NOT found" : "DELETED";
    smsc_log_debug(rplog, "execCommand(ctx): profile key=%s %s in local storage",
                    profKey.c_str(), err_str.c_str());
    GetProfileCmd get_profile_cmd(profKey);
    if (!sendCommandToCP(get_profile_cmd)) {
      smsc_log_warn(rplog, "execCommand(ctx): can't send command to CP");
      createResponseForClient(&osb, RESPONSE_ERROR);
      return;
    }
    if (ctx.batch) {
      ++ctx.batch_cmd_count;
    }
    AbntAddr addr(profKey.c_str());
    commands.Insert(addr, CmdContext(cmd, isb, &osb, ctx.socket));
    return;
  }
  switch (pf->getState()) {
  case LOCKED:
    smsc_log_debug(rplog, "execCommand(ctx): profile key=%s LOCKED",
                    profKey.c_str());
    createResponseForClient(&osb, RESPONSE_ERROR);
    break;
  case OK:
    smsc_log_debug(rplog, "execCommand(ctx): profile key=%s found int local storage", 
                   profKey.c_str());
    execCommand(cmd, pf, profKey, isb, osb);
    break;
  default:
    smsc_log_debug(rplog, "execCommand(ctx): profile key=%s unknown state",
                    profKey.c_str());
    createResponseForClient(&osb, RESPONSE_ERROR);
  }
/*
  AbntAddr addr(profKey.c_str());
  uint8_t pf_state = OK;
  profiles_states.Get(addr, pf_state);
  if (pf_state != OK) {
    smsc_log_debug(rplog, "execCommand(ctx): profile key=%s LOCKED",
                    profKey.c_str());
    createResponseForClient(&osb, RESPONSE_ERROR);
    return;
  }
  Profile *pf = getProfile(profKey);
  if (pf) {
    smsc_log_debug(rplog, "execCommand(ctx): profile key=%s found int local storage", 
                   profKey.c_str());
    execCommand(cmd, pf, profKey, isb, osb);
    return;
  }
  smsc_log_debug(rplog, "execCommand(ctx): profile key=%s NOT found int local storage",
                  profKey.c_str());
  GetProfileCmd get_profile_cmd(profKey);
  if (!sendCommandToCP(get_profile_cmd)) {
    smsc_log_warn(rplog, "execCommand(ctx): can't send command to CP");
    createResponseForClient(&osb, RESPONSE_ERROR);
    return;
  }
  if (ctx.batch) {
    ++ctx.batch_cmd_count;
  }
  commands.Insert(addr, CmdContext(cmd, isb, &osb, ctx.socket));
*/
}
  
bool RegionPersServer::execCommand(PersCmd cmd, Profile *pf, const string& str_key,
                                    SerialBuffer& isb, SerialBuffer& osb){
  smsc_log_debug(rplog, "execCommand");
  string name;
  Property prop;
  //osb.SetPos(4);
  switch(cmd) {
  case PC_DEL:
    isb.ReadString(name);
    DelCmdHandler(pf, str_key, name, osb);
    break;
  case PC_SET:
    prop.Deserialize(isb);
    SetCmdHandler(pf, str_key, prop, osb);
    break;
  case PC_GET:
    isb.ReadString(name);
    return GetCmdHandler(pf, str_key, name, osb);
  case PC_INC:
    prop.Deserialize(isb);
    IncCmdHandler(pf, str_key, prop, osb);
    break;
  case PC_INC_MOD:
  {
    int inc = isb.ReadInt32();
    prop.Deserialize(isb);
    IncModCmdHandler(pf, str_key, prop, inc, osb);
    break;
  }
  default:
    smsc_log_debug(rplog, "execCommand: Bad command %d", cmd);
  }
  return true;
  //SetPacketSize(osb);
}

void RegionPersServer::onDisconnect(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "onDisconnect");
  if (!connected) {
    return;
  }
  ConnectionContext *central_ctx = (ConnectionContext *)central_socket->getData(0);
  if (!central_ctx) {
    connected = false;
    smsc_log_debug(rplog, "onDisconnect: central_socket hasn't ctx");
  }
  if (central_ctx == &ctx) {
    connected = false;
    smsc_log_warn(rplog, "onDisconnect: central_socket disconnected");
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
  ctx->outbuf.setPos(0);
  listener.addRW(central_socket);
  return true;
}

void RegionPersServer::getProfileCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "getProfileCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  GetProfileCmd get_profile_cmd(isb);
  smsc_log_debug(rplog, "getProfileCmdHandler: key=%s", get_profile_cmd.key.c_str());
  Profile *pf = getProfile(get_profile_cmd.key);
  ProfileRespCmd profile_resp(get_profile_cmd.key);
  if (!pf) {
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler: profile key=%s not found ", get_profile_cmd.key.c_str());
    sendCommandToCP(profile_resp);
    return;
  }
  AbntAddr addr(get_profile_cmd.key.c_str());
  //addr.setNumberingPlan(1);
  //addr.setTypeOfNumber(1);
  //uint8_t pf_state = OK;
  //profiles_states.Get(addr, pf_state);
  switch (pf->getState()) {
  case LOCKED : 
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler: profile key=%s LOCKED", get_profile_cmd.key.c_str());
    break;
  case DELETED :
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler: profile key=%s DELETED", get_profile_cmd.key.c_str());
    break;
  case OK : 
    //profiles_states.Insert(addr, LOCKED);
    requested_profiles.Insert(addr, time(NULL));
    smsc_log_debug(rplog, "getProfileCmdHandler: set profile key=%s LOCKED", get_profile_cmd.key.c_str());
    profile_resp.setProfile(pf);
    //
    pf->setLocked();
    getAbonentStore()->storeProfile(addr, pf);
    break;
  default:
    profile_resp.is_ok = 0;
    smsc_log_warn(rplog, "getProfileCmdHandler: profile key=%s unknown state", get_profile_cmd.key.c_str());
  }
  sendCommandToCP(profile_resp);
}

void RegionPersServer::profileRespCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "profileRespCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  ProfileRespCmd profile_resp(isb);
  smsc_log_debug(rplog, "profileRespCmdHandler: profile key=%s", profile_resp.key.c_str());
  CmdContext* cmd_ctx;
  AbntAddr addr(profile_resp.key.c_str());
  //addr.setNumberingPlan(1);
  //addr.setTypeOfNumber(1);
  if (!(cmd_ctx = commands.GetPtr(addr))) {
    smsc_log_warn(rplog, "profileRespCmdHandler: command conntext key=%s not found",
                    addr.toString().c_str());
    //послать Done Error, т.к. это означает что пришёл профиль который платформа не запрашивала
    DoneCmd done(0, profile_resp.key);
    sendCommandToCP(done);
    return;
  } else {
    cmd_ctx->last_cmd_id = CentralPersCmd::PROFILE_RESP;
    cmd_ctx->start_time = time(NULL);
  }
  if (!profile_resp.is_ok) {
    smsc_log_warn(rplog, "profileRespCmdHandler: receive PROFILE_RESP ERROR key=%s",
                   profile_resp.key.c_str());
    sendResponseError(cmd_ctx, profile_resp.key);
    return;
  }
  Profile* profile = getAbonentStore()->createProfile(addr);
  if (!profile) {
    smsc_log_warn(rplog, "profileRespCmdHandler: can't create new profile");
    sendResponseError(cmd_ctx, profile_resp.key);
    return;
  }
  if (profile_resp.getProfile()) {
    smsc_log_debug(rplog, "profileRespCmdHandler: response has profile");
    profile_resp.getProfile()->copyPropertiesTo(profile);
    //
    profile->setLocked();
    getAbonentStore()->storeProfile(addr, profile);
    //profiles_states.Insert(addr, LOCKED);
    DoneCmd done(1, profile_resp.key);
    sendCommandToCP(done);
    return;
  }
  smsc_log_debug(rplog, "profileRespCmdHandler: response has NOT profile");
  /*
  как будет выглядеть остаток фу-ии если будет подтверждение при регистрации профиля
  getAbonentStore()->storeProfile(addr, profile);
  profiles_states.Insert(addr, LOCKED);
  DoneCmd done(1, profile_resp.key);
  sendCommandToCP(done);
  */
  bool result = false;
  if (cmd_ctx && listener.exists(cmd_ctx->socket)) {
    result = execCommand(cmd_ctx->cmd_id, profile, profile_resp.key, cmd_ctx->isb, *cmd_ctx->osb);
    sendCommandToClient(cmd_ctx);
  } else {
    smsc_log_warn(rplog, "profileRespCmdHandler: client connection error key=%s ctx=%p socket %p disconnected",
                    profile_resp.key.c_str(), &ctx, cmd_ctx->socket);
  }
  if (!result) {
    getAbonentStore()->storeProfile(addr, profile);
  }
  commands.Delete(addr);
}

void RegionPersServer::doneCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "doneCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  DoneCmd done(isb);
  smsc_log_debug(rplog, "doneCmdHandler: profile key=%s", done.key.c_str());
  AbntAddr addr(done.key.c_str());
  //addr.setNumberingPlan(1);
  //addr.setTypeOfNumber(1);
  //profiles_states.Delete(addr);
  requested_profiles.Delete(addr);
  DoneRespCmd done_resp(1, done.key);
  if (done.is_ok) {
    smsc_log_debug(rplog, "doneCmdHandler: receive DONE OK, delete profile key=%s", addr.toString().c_str());
    getAbonentStore()->deleteProfile(addr);
    sendCommandToCP(done_resp);
    //profiles_states.Insert(addr, DELETED);
  } else {
    smsc_log_debug(rplog, "doneCmdHandler: receive DONE ERROR");
    //
    Profile *pf = getProfile(done.key);
    if (!pf) {
      smsc_log_debug(rplog, "doneCmdHandler: profile key=%s not found", addr.toString().c_str());
    }
    pf->setOk();
    getAbonentStore()->storeProfile(addr, pf);

    //done_resp.is_ok = 0;
     //profiles_states.Insert(addr, OK);
    //что делать если пришёл DONE=ERROR
  }
}

void RegionPersServer::doneRespCmdHandler(ConnectionContext &ctx) {
  smsc_log_debug(rplog, "doneRespCmdHandler");
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  DoneRespCmd done_resp(isb);
  smsc_log_debug(rplog, "doneRespCmdHandler: profile key=%s", done_resp.key.c_str());
  AbntAddr addr(done_resp.key.c_str());
  //addr.setNumberingPlan(1);
  //addr.setTypeOfNumber(1);
  CmdContext cmd_ctx;
  if (!(commands.Get(addr, cmd_ctx))) {
    smsc_log_warn(rplog, "doneRespCmdHandler: command conntext key=%s not found ctx=%p",
                    addr.toString().c_str(), &ctx);
    return;
  }
  bool client_connected = listener.exists(cmd_ctx.socket);
  if (!client_connected) {
    smsc_log_warn(rplog, "doneRespCmdHandler: client connection error key=%s ctx=%p socket %p disconnected",
                    addr.toString().c_str(), &ctx, cmd_ctx.socket);
  }
  //profiles_states.Delete(addr);
  if (done_resp.is_ok) {
    //profiles_states.Insert(addr, OK);
    Profile* profile = getProfile(done_resp.key);
    if (!profile) {
      smsc_log_debug(rplog, "doneRespCmdHandler: profile key=%s not found ctx=%p",
                      addr.toString().c_str(), &ctx);
      if (client_connected) {
        createResponseForClient(cmd_ctx.osb, RESPONSE_ERROR);
        sendCommandToClient(&cmd_ctx);
      }
      commands.Delete(addr);
      return;
    }
    //
    profile->setOk();
    bool result = false;
    if (client_connected) {
      result = execCommand(cmd_ctx.cmd_id, profile, done_resp.key, cmd_ctx.isb, *cmd_ctx.osb);
      sendCommandToClient(&cmd_ctx);
    }
    if (!result) {
      getAbonentStore()->storeProfile(addr, profile);
    }
  } else {
    smsc_log_warn(rplog, "doneRespCmdHandler: receive ERROR DONE_RESP key=%s", done_resp.key.c_str());
    //profiles_states.Insert(addr, DELETED);
    getAbonentStore()->deleteProfile(addr);
    smsc_log_debug(rplog, "doneRespCmdHandler: delete profile key=%s", addr.toString().c_str());
    if (client_connected) {
      createResponseForClient(cmd_ctx.osb, RESPONSE_ERROR);
      sendCommandToClient(&cmd_ctx);
    }
  }
  commands.Delete(addr);
}

void RegionPersServer::checkOwnCmdHandler(ConnectionContext &ctx) {
}

void RegionPersServer::DelCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb) {
  IntProfileStore *is;
  bool exists = false;
  if(is = findStore(pt)) {
    smsc_log_debug(rplog, "DelCmdHandler store=%d, key=%d, name=%s", pt, int_key, name.c_str());
    exists = is->delProperty(int_key, name.c_str());
  }
  SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void RegionPersServer::GetCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb) {
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
  IntProfileStore *is;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "SetCmdHandler store=%d, key=%d, prop=%s", pt, int_key, prop.toString().c_str());
    is->setProperty(int_key, prop);
  }
  SendResponse(osb, RESPONSE_OK);
}

void RegionPersServer::IncCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb) {
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
  bool exists = getAbonentStore()->delProperty(pf, str_key.c_str(), name.c_str());
  SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

bool RegionPersServer::GetCmdHandler(Profile *pf, const string& str_key,
                                const string& name, SerialBuffer& osb) {
  Property prop;
  smsc_log_debug(rplog, "GetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), name.c_str());
  if(getAbonentStore()->getProperty(pf, str_key.c_str(), name.c_str(), prop)) {   
    smsc_log_debug(rplog, "GetCmdHandler prop=%s", prop.toString().c_str());
    SendResponse(osb, RESPONSE_OK);
    prop.Serialize(osb);
    if (prop.getTimePolicy() != R_ACCESS) {
      return false;
    }
  } else {
    smsc_log_debug(rplog, "GetCmdHandler property not found: AbonentStore key=%s, name=%s", str_key.c_str(), name.c_str());    
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
  }        
  return true;
}

void RegionPersServer::SetCmdHandler(Profile *pf, const string& str_key,
                                Property& prop, SerialBuffer& osb) {
  smsc_log_debug(rplog, "SetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.toString().c_str());
  getAbonentStore()->setProperty(pf, str_key.c_str(), prop);
  SendResponse(osb, RESPONSE_OK);
}

void RegionPersServer::IncCmdHandler(Profile *pf, const string& str_key,
                                Property& prop, SerialBuffer& osb) {
  smsc_log_debug(rplog, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName().c_str());
  bool exists = getAbonentStore()->incProperty(pf, str_key.c_str(), prop);
  SendResponse(osb, exists ? RESPONSE_OK : RESPONSE_PROPERTY_NOT_FOUND);
}

void RegionPersServer::IncModCmdHandler(Profile *pf, const string& str_key,
                                         Property& prop, int mod, SerialBuffer& osb) {
  int res = 0;
  smsc_log_debug(rplog, "IncModCmdHandler AbonentStore: key=%s, name=%s, mod=%d", str_key.c_str(), prop.getName().c_str(), mod);
  if(getAbonentStore()->incModProperty(pf, str_key.c_str(), prop, mod, res)) {
    SendResponse(osb, RESPONSE_OK);
    osb.WriteInt32(res);
  } else {
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
  }
}

void RegionPersServer::createResponseForClient(SerialBuffer* sb, PersServerResponseType r) {
  if (!sb) {
    smsc_log_warn(rplog, "createResponseForClient: serial buffer error");
    return;
  }
  smsc_log_debug(rplog, "createResponseForClient: response=%d", static_cast<int>(r));
  //sb->SetPos(4);
  SendResponse(*sb, r);
  //SetPacketSize(*sb);
}

void RegionPersServer::sendResponseError(CmdContext* cmd_ctx, const string& key) {
  if (cmd_ctx && listener.exists(cmd_ctx->socket)) {
    createResponseForClient(cmd_ctx->osb, RESPONSE_ERROR);
    sendCommandToClient(cmd_ctx);
  } else {
    smsc_log_error(rplog, "sendResponseError: socket %p disconnected", key.c_str(), cmd_ctx->socket);
  }
  DoneCmd done(0, key);
  sendCommandToCP(done);
  AbntAddr addr(key.c_str());
  //addr.setNumberingPlan(1);
  //addr.setTypeOfNumber(1);
  commands.Delete(addr);
}

void RegionPersServer::sendCommandToClient(CmdContext* cmd_ctx) {
  if (!cmd_ctx || !cmd_ctx->osb) {
    smsc_log_warn(rplog, "sendCommandToClient: serial buffer error in ctx=%p", &cmd_ctx);
    return;
  }
  if (!listener.exists(cmd_ctx->socket)) {
    smsc_log_warn(rplog, "sendCommandToClient: client connection error socket %p desconnectd",
                  cmd_ctx->socket);
    return;
  }
  ConnectionContext* ctx = (ConnectionContext*)cmd_ctx->socket->getData(0);
  if (!ctx) {
    smsc_log_warn(rplog, "sendCommandToClient: empty connection context");
    return;
  }
  smsc_log_debug(rplog, "sendCommandToClient: outbuf length=%d", ctx->outbuf.GetSize());
  if (!ctx->batch || !ctx->batch_cmd_count) {
    SetPacketSize(ctx->outbuf);
    ctx->outbuf.SetPos(0);
    listener.addRW(cmd_ctx->socket);
    return;
  }
  --ctx->batch_cmd_count;
  smsc_log_debug(rplog, "sendCommandToClient: batch commands count = %d", ctx->batch_cmd_count);
  if (ctx->batch_cmd_count == 0) {
    ctx->batch = false;
    smsc_log_debug(rplog, "sendCommandToClient : send batch");
    SetPacketSize(ctx->outbuf);
    ctx->outbuf.SetPos(0);
    listener.addRW(cmd_ctx->socket);
  }
}

void RegionPersServer::commandTimeout(const AbntAddr& addr, CmdContext& ctx) {
   if (ctx.last_cmd_id == CentralPersCmd::GET_PROFILE) {
     smsc_log_warn(rplog, "commandTimeout: PROFILE_RESP timeout key=%s", addr.toString().c_str());
     //string key(addr.toString(), ADDR_PREFIX_SIZE);
     string key(addr.toString());
     sendResponseError(&ctx, key);
     //return;
   }
   if (ctx.last_cmd_id == CentralPersCmd::PROFILE_RESP) {
     smsc_log_warn(rplog, "commandTimeout: DONE_RESP timeout key=%s", addr.toString().c_str());
     //не пришёл DONE_RESP
     commands.Delete(addr);
   }
   profiles_states.Delete(addr);
}

void RegionPersServer::doneTimeout(const AbntAddr& addr) {
  smsc_log_warn(rplog, "doneTimeout: DONE timeout key=%s", addr.toString().c_str());
  //string key(addr.toString(), ADDR_PREFIX_SIZE);
  string key(addr.toString());
  DoneRespCmd done_resp(0, key);
  sendCommandToCP(done_resp);
  requested_profiles.Delete(addr);
  profiles_states.Delete(addr);
}

void RegionPersServer::checkTimeouts()
{
	time_t time_bound = time(NULL) - timeout;
	int i = 1;
	smsc_log_debug(rplog, "Checking timeouts...");
	while(i < listener.count())
	{
		Socket* s = listener.get(i);
		ConnectionContext* ctx = (ConnectionContext*)s->getData(0);
		if(ctx->lastActivity  < time_bound)
		{
          if (s == central_socket) {
            smsc_log_debug(rplog, "central socket inactive");		
            if (!pingCP()) {
              removeSocket(s, i);
            }
            ++i;
          } else {
            smsc_log_info(rplog, "Disconnecting inactive user by timeout");		
            removeSocket(s, i);
          }
		}
		else
			i++;
	}
    if (isStopping) {
      return;
    }
    if (!connected) {
      bindToCP(); 
    }
}

void RegionPersServer::checkTransactionsTimeouts() {
  if (isStopped()) {
    return;
  }
  smsc_log_debug(rplog, "Checking transactions timeouts...");
  time_t time_bound = time(NULL) - transactTimeout;
  AbntAddr key;
  CmdContext ctx;
  commands.First();
  while (commands.Next(key, ctx)) {
    if (ctx.start_time < time_bound) {
      commandTimeout(key, ctx);
    }
  }
  time_t request_start_time;
  requested_profiles.First();
  while (requested_profiles.Next(key, request_start_time)) {
    if (request_start_time < time_bound) {
      doneTimeout(key);
    }
  }
  //last_check_time = time(NULL);
}

bool RegionPersServer::pingCP() {
  if (!connected) {
    return false;
  }
  try {
    SerialBuffer sb;
    sb.SetPos(PACKET_LENGTH_SIZE);
    sb.WriteInt8(CentralPersCmd::PING);
    sb.SetPos(0);
    sb.WriteInt32(sb.GetSize());
    sb.SetPos(0);
    WriteAllToCP(sb.c_ptr(), sb.length());
    smsc_log_debug(rplog, "Ping sent length=%d data=%s", sb.length(), sb.toString().c_str());			
    sb.Empty();
    char buf[1024];
    ReadAllFromCP(buf, PACKET_LENGTH_SIZE);
    sb.Append(buf, PACKET_LENGTH_SIZE);
    sb.SetPos(0);
    uint32_t size = sb.ReadInt32() - PACKET_LENGTH_SIZE;
    if (size != sizeof(uint8_t)) {
      smsc_log_warn(rplog, "Ping failed packet size=%d", size);
      return false;
    }
    ReadAllFromCP(buf, sizeof(uint8_t));
    sb.Append(buf, sizeof(uint8_t));
    sb.SetPos(PACKET_LENGTH_SIZE);
    if (sb.ReadInt8() != CentralPersCmd::PING_OK) {
      smsc_log_warn(rplog, "Ping failed");
      return false;
    }
    smsc_log_debug(rplog, "Ping OK");			
    return true;
  } catch (const Exception& e) {
    smsc_log_warn(rplog, "Ping failed: %s", e.what());
    return false;
  } catch (const SerialBufferOutOfBounds &e) {
    smsc_log_warn(rplog, "Ping failed: SerialBufferOutOfBounds bad data in buffer");
    return false;
  }
}

void RegionPersServer::ReadAllFromCP(char* buf, uint32_t sz)
{
    int cnt;
    uint32_t rd = 0;
    struct pollfd pfd;

    while(sz)
    {
		pfd.fd = central_socket->getSocket();
		pfd.events = POLLIN;
        //if(poll(&pfd, 1, timeout * 1000) <= 0 || !(pfd.revents & POLLIN))
        if(poll(&pfd, 1, timeout) <= 0 || !(pfd.revents & POLLIN))
            throw Exception("timeout");

        cnt = central_socket->Read(buf + rd, sz);
        if(cnt <= 0)
            throw Exception("read failed");

        rd += cnt;
        sz -= cnt;
    }
}

void RegionPersServer::WriteAllToCP(const char* buf, uint32_t sz)
{
    int cnt;
    uint32_t wr = 0;
    struct pollfd pfd;    

    while(sz)
    {
		pfd.fd = central_socket->getSocket();
		pfd.events = POLLOUT | POLLIN;
        //if(poll(&pfd, 1, timeout * 1000) <= 0)
        if(poll(&pfd, 1, timeout) <= 0)
          throw Exception("timeout");
		else if(!(pfd.revents & POLLOUT) || (pfd.revents & POLLIN))
          throw Exception("send failed");

        cnt = central_socket->Write(buf + wr, sz);
        if(cnt <= 0)
          throw Exception("send failed");

        wr += cnt;
        sz -= cnt;
    }
}

CmdContext::CmdContext():cmd_id(PC_UNKNOWN), osb(0), socket(0),
                         start_time(time(NULL)), last_cmd_id(CentralPersCmd::GET_PROFILE) {};

CmdContext::CmdContext(PersCmd _cmd_id, SerialBuffer& _isb, SerialBuffer* _osb, Socket *s) :
                       cmd_id(_cmd_id), osb(_osb), socket(s),
                       start_time(time(NULL)), last_cmd_id(CentralPersCmd::GET_PROFILE) {
  isb.blkwrite(_isb.c_curPtr(), _isb.length() - _isb.getPos());
  isb.setPos(0);
}

CmdContext::CmdContext(const CmdContext& ctx):cmd_id(ctx.cmd_id), osb(ctx.osb), socket(ctx.socket),
                                              start_time(ctx.start_time), last_cmd_id(ctx.last_cmd_id){
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
  socket = ctx.socket;
  start_time = ctx.start_time;
  last_cmd_id = ctx.last_cmd_id;
  return *this;
}


}//pers
}//scag

