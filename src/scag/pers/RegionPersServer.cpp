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
  info_log = Logger::getInstance("rp.abnt");
  smsc_log_info(rplog, "transactTimeout=%d", transactTimeout);
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
    smsc_log_info(rplog, "region perserver bound to CP socket:%p cxt:%p",
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
  if (!connected) {
    return processPacketFromClient(ctx);
  }
  ConnectionContext *central_ctx = (ConnectionContext *)central_socket->getData(0);
  if (!central_ctx) {
    connected = false;
    removeSocket(central_socket);
    smsc_log_debug(rplog, "processPacket: central_socket hasn't ctx");
    return processPacketFromClient(ctx);
  }
  if (central_ctx == &ctx) {
    try {
      smsc_log_debug(rplog, "Start process packet %p from CP", &ctx);
      ctx.inbuf.setPos(PACKET_LENGTH_SIZE);
      processPacketFromCP(ctx);
      smsc_log_debug(rplog, "End process packet from CP");
    } catch(const SerialBufferOutOfBounds &e) {
      smsc_log_debug(rplog, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s",
                      ctx.inbuf.length(), ctx.inbuf.toString().c_str());
      ctx.inbuf.Empty();
    }
    catch(const std::runtime_error& e) {
      smsc_log_debug(rplog, "Error profile key: %s", e.what());
    }
    catch(const FileException& e) {
      smsc_log_warn(rplog, "FileException: '%s'. received buffer len=%d, data=%s",
                     e.what(), ctx.inbuf.length(), ctx.inbuf.toString().c_str());
    }
    catch(const std::exception& e) {
      smsc_log_warn(rplog, "std::exception: %s. received buffer len=%d, data=%s",
                     e.what(), ctx.inbuf.length(), ctx.inbuf.toString().c_str());
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
  //smsc_log_debug(rplog, "process packet %p from CP", &ctx);
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;

    uint8_t cmd = isb.ReadInt8();

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

    case CentralPersCmd::CHECK_OWN_RESP:
      smsc_log_debug(rplog, "CHECK_OWN_RESP received");
      checkOwnRespCmdHandler(ctx);
      break;
    }

  if (ctx.inbuf.getPos() < ctx.inbuf.GetSize()) {
    smsc_log_debug(rplog, "continue processing packet %p from CP inbuf pos=%d length=%d",
                   &ctx, isb.getPos(), isb.GetSize());
    processPacketFromCP(ctx);
  }
  return true;
}

bool RegionPersServer::processPacketFromClient(ConnectionContext& ctx) {
  smsc_log_debug(rplog, "Start process packet %p from client", &ctx);
  SerialBuffer &isb = ctx.inbuf;
  PersServerResponseType response = RESPONSE_ERROR;
  try {
    ctx.outbuf.SetPos(PACKET_LENGTH_SIZE);
    execCommand(ctx);
    if (!(ctx.batch && ctx.batch_cmd_count) && ctx.outbuf.GetPos() > PACKET_LENGTH_SIZE) {
      SetPacketSize(ctx.outbuf);
    }
    return true;
  } catch(const SerialBufferOutOfBounds &e) {
    smsc_log_warn(rplog, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s, pos=%d", isb.length(), isb.toString().c_str(), isb.getPos());
    response = RESPONSE_BAD_REQUEST;
  }
  catch(const std::runtime_error& e) {
    smsc_log_warn(rplog, "Error profile key: %s", e.what());
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
    smsc_log_warn(rplog, "Bad data in buffer received len=%d, data=%s", isb.length(), isb.toString().c_str());
  }
  if (ctx.transact_batch) {
    rollbackCommands(response);
  }
  createResponseForClient(&ctx.outbuf, response, "");
  SetPacketSize(ctx.outbuf);
  return true;
}

PersServerResponseType RegionPersServer::execCommand(PersCmd cmd, ProfileType pt,
                                                      SerialBuffer& isb, SerialBuffer& osb) {
  uint32_t int_key = isb.ReadInt32();
  string name;
  Property prop;
  //osb.SetPos(4);
  switch(cmd) {
  case PC_DEL:
    isb.ReadString(name);
    return DelCmdHandler(pt, int_key, name, osb);
  case PC_SET:
    prop.Deserialize(isb);
    return SetCmdHandler(pt, int_key, prop, osb);
  case PC_GET:
    isb.ReadString(name);
    return GetCmdHandler(pt, int_key, name, osb);
  case PC_INC:
    prop.Deserialize(isb);
    return IncCmdHandler(pt, int_key, prop, osb);
  case PC_INC_RESULT:
    prop.Deserialize(isb);
    return IncResultCmdHandler(pt, int_key, prop, osb);
  case PC_INC_MOD:
  {
    int inc = isb.ReadInt32();
    prop.Deserialize(isb);
    return IncModCmdHandler(pt, int_key, prop, inc, osb);
  }
  default:
    smsc_log_debug(rplog, "Bad command %d", cmd);
    return RESPONSE_BAD_REQUEST;
  }
}

void RegionPersServer::skipCommand(PersCmd cmd, SerialBuffer& isb) {
  string name;
  Property prop;
  switch(cmd) {
  case PC_DEL:
    isb.ReadString(name);
    break;
  case PC_SET:
    prop.Deserialize(isb);
    break;
  case PC_GET:
    isb.ReadString(name);
    break;
  case PC_INC:
    prop.Deserialize(isb);
    break;
  case PC_INC_RESULT:
    prop.Deserialize(isb);
    break;
  case PC_INC_MOD:
    isb.ReadInt32();
    prop.Deserialize(isb);
    break;
  }
}

PersServerResponseType RegionPersServer::execCommand(ConnectionContext& ctx) {
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  PersCmd cmd;
  ProfileType pt;
  string str_key;
  cmd = (PersCmd)isb.ReadInt8();
  if(cmd == PC_PING) {
    smsc_log_debug(rplog, "Ping received");
    createResponseForClient(&osb, RESPONSE_OK, "");
    return RESPONSE_OK;
  }
  if (cmd == PC_BATCH) {
    uint16_t cnt = isb.ReadInt16();
    ctx.batch = true;
    ctx.batch_cmd_count = cnt;
    smsc_log_debug(rplog, "Batch start. Commands count=%d", cnt);
    while(cnt--) {
      if (execCommand(ctx) == COMMAND_IN_PROCESS) {
        smsc_log_debug(rplog, "execCommand: Batch command in process");
        return RESPONSE_OK;
      }
    }
    ctx.cancelBatch();
    smsc_log_debug(rplog, "Batch end");
    return RESPONSE_OK;
  }
  if (cmd == PC_TRANSACT_BATCH) {
    uint16_t cnt = isb.ReadInt16();
    ctx.batch = true;
    ctx.batch_cmd_count = cnt;
    ctx.transact_batch = true;
    setNeedBackup(true);
    smsc_log_debug(rplog, "Transact Batch start. Commands count=%d", cnt);
    while(cnt--) {
      PersServerResponseType result = execCommand(ctx);
      if (result == COMMAND_IN_PROCESS) {
        smsc_log_debug(rplog, "execCommand: Batch command in process");
        setNeedBackup(false);
        return RESPONSE_OK;
      } else if (result != RESPONSE_OK) {
        rollbackCommands(result);
        ctx.cancelBatch();
        return result;
      }
    }
    resetStroragesBackup();
    ctx.cancelBatch();
    smsc_log_debug(rplog, "Batch end");
    return RESPONSE_OK;
  }

  if (ctx.batch && ctx.batch_cmd_count) {
    --ctx.batch_cmd_count;
  }
  pt = (ProfileType)isb.ReadInt8();
  smsc_log_debug(rplog, "profile type=%d", pt);

  if(pt != PT_ABONENT) {
    return execCommand(cmd, pt, isb, osb);
  } 
  //TODO: remove getProfileKey method
  isb.ReadString(str_key);
  string profKey = getProfileKey(str_key);
  AbntAddr addr(profKey.c_str());
  if (commands.GetPtr(addr)) {
    smsc_log_debug(info_log, "profile key=%s in process", profKey.c_str());
    createResponseForClient(&osb, RESPONSE_PROFILE_LOCKED, profKey);
    skipCommand(cmd, isb);
    return RESPONSE_PROFILE_LOCKED;
  }

//
  Profile *pf = getProfile(profKey);
  if (!pf || pf->getState() == DELETED) {
    string err_str = !pf ? "not found" : "deleted";
    smsc_log_info(info_log, "profile key=%s %s in local storage",
                    profKey.c_str(), err_str.c_str());
    //AbntAddr addr(profKey.c_str());
    //if (commands.GetPtr(addr)) {
      //smsc_log_info(info_log, "profile key=%s in process", profKey.c_str());
      //createResponseForClient(&osb, RESPONSE_PROFILE_LOCKED, profKey);
      //skipCommand(cmd, isb);
      //return RESPONSE_PROFILE_LOCKED;
    //}
    GetProfileCmd get_profile_cmd(profKey);
    if (!sendCommandToCP(get_profile_cmd)) {
      smsc_log_warn(rplog, "can't send command to CP");
      createResponseForClient(&osb, RESPONSE_ERROR, profKey);
      skipCommand(cmd, isb);
      return RESPONSE_ERROR;
    }
    smsc_log_info(info_log, "profile key=%s request to CP was sent", profKey.c_str());
    if (ctx.batch) {
      ++ctx.batch_cmd_count;
    }
    commands.Insert(addr, CmdContext(cmd, isb, &osb, ctx.socket, ctx.transact_batch));
    skipCommand(cmd, isb);
    return COMMAND_IN_PROCESS;
  }
  switch (pf->getState()) {
  case OK:
    smsc_log_debug(info_log, "profile key=%s found in local storage", profKey.c_str());
    return execCommand(cmd, pf, profKey, isb, osb);
  case LOCKED: {
    smsc_log_warn(info_log, "profile key=%s state=LOCKED, checking ownership", profKey.c_str());
    CheckOwnCmd check_own(profKey);
    sendCommandToCP(check_own);
    createResponseForClient(&osb, RESPONSE_PROFILE_LOCKED, profKey);
    skipCommand(cmd, isb);
    return  RESPONSE_PROFILE_LOCKED;
  }
  default:
    smsc_log_info(info_log, "profile key=%s has unknown state", profKey.c_str());
    createResponseForClient(&osb, RESPONSE_ERROR, profKey);
    skipCommand(cmd, isb);
    return RESPONSE_ERROR;
  }
}
  
PersServerResponseType RegionPersServer::execCommand(PersCmd cmd, Profile *pf, const string& str_key,
                                    SerialBuffer& isb, SerialBuffer& osb, bool alwaysStore){
  string name;
  Property prop;
  //osb.SetPos(4);
  switch(cmd) {
  case PC_DEL:
    isb.ReadString(name);
    return DelCmdHandler(pf, str_key, name, osb, alwaysStore);
  case PC_SET:
    prop.Deserialize(isb);
    return SetCmdHandler(pf, str_key, prop, osb, alwaysStore);
  case PC_GET:
    isb.ReadString(name);
    return GetCmdHandler(pf, str_key, name, osb, alwaysStore);
  case PC_INC:
    prop.Deserialize(isb);
    return IncCmdHandler(pf, str_key, prop, osb, alwaysStore);
  case PC_INC_RESULT:
    prop.Deserialize(isb);
    return IncResultCmdHandler(pf, str_key, prop, osb, alwaysStore);
  case PC_INC_MOD:
  {
    int inc = isb.ReadInt32();
    prop.Deserialize(isb);
    return IncModCmdHandler(pf, str_key, prop, inc, osb, alwaysStore);
  }
  default:
    smsc_log_debug(rplog, "execCommand: Bad command %d", cmd);
    if (alwaysStore) {
      AbntAddr addr(str_key.c_str());
      getAbonentStore()->storeProfile(addr, pf);
    }
    return RESPONSE_BAD_REQUEST;
  }
  //return true;
  //SetPacketSize(osb);
}

void RegionPersServer::onDisconnect(ConnectionContext &ctx) {
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
  smsc_log_debug(rplog, "send %s to CP", cmd.name.c_str());
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
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  GetProfileCmd get_profile_cmd(isb);
  Profile *pf = getProfile(get_profile_cmd.key);
  ProfileRespCmd profile_resp(get_profile_cmd.key);
  smsc_log_info(info_log, "profile key=%s request received from CP",get_profile_cmd.key.c_str());
  if (!pf) {
    profile_resp.setStatus(scag::cpers::STATUS_ERROR);
    smsc_log_warn(info_log, "profile key=%s not found ", get_profile_cmd.key.c_str());
    //smsc_log_warn(rplog, "getProfileCmdHandler: profile key=%s not found ", get_profile_cmd.key.c_str());
    sendCommandToCP(profile_resp);
    return;
  }
  switch (pf->getState()) {
  case LOCKED : {
    smsc_log_warn(info_log, "profile key=%s state=LOCKED", get_profile_cmd.key.c_str());
    AbntAddr addr(get_profile_cmd.key.c_str());
    commands.Insert(addr, CmdContext());
    profile_resp.setStatus(scag::cpers::STATUS_OK);
    profile_resp.setProfile(pf);
    break;
  }
  case DELETED :
    profile_resp.setStatus(scag::cpers::STATUS_ERROR);
    smsc_log_warn(info_log, "profile key=%s state=DELETED", get_profile_cmd.key.c_str());
    break;
  case OK : {
    AbntAddr addr(get_profile_cmd.key.c_str());
    commands.Insert(addr, CmdContext());
    smsc_log_info(info_log, "profile key=%s set state=LOCKED", get_profile_cmd.key.c_str());
    profile_resp.setStatus(scag::cpers::STATUS_OK);
    profile_resp.setProfile(pf);
    pf->setLocked();
    getAbonentStore()->storeProfile(addr, pf);
    break;
  }
  default:
    profile_resp.setStatus(scag::cpers::STATUS_ERROR);
    smsc_log_warn(info_log, "profile key=%s has unknownb state", get_profile_cmd.key.c_str());
  }
  sendCommandToCP(profile_resp);
}

void RegionPersServer::profileRespCmdHandler(ConnectionContext &ctx) {
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  ProfileRespCmd profile_resp(isb);
  CmdContext* cmd_ctx;
  AbntAddr addr(profile_resp.key.c_str());
  smsc_log_debug(info_log, "profile key=%s response received from CP", profile_resp.key.c_str());
  if (!(cmd_ctx = commands.GetPtr(addr))) {
    smsc_log_warn(rplog, "profileRespCmdHandler: command conntext key=%s not found",
                    addr.toString().c_str());
    //послать Done Error, т.к. это означает что пришёл профиль который платформа не запрашивала
    DoneCmd done(0, profile_resp.key);
    sendCommandToCP(done);
    return;
  } else {
    cmd_ctx->wait_cmd_id = CentralPersCmd::DONE_RESP;
    cmd_ctx->start_time = time(NULL);
  }
  if (!profile_resp.isOk()) {
    PersServerResponseType response = RESPONSE_ERROR;
    string status_text = "ERROR";
    if (profile_resp.getStatus() == scag::cpers::STATUS_LOCKED) {
      response = RESPONSE_PROFILE_LOCKED;
      status_text = "LOCKED";
    }
    smsc_log_warn(info_log, "profile key=%s response=%s", profile_resp.key.c_str(), status_text.c_str());
    sendResponseError(cmd_ctx, profile_resp.key, false, response);
    return;
  }
  Profile* profile = getAbonentStore()->createProfile(addr);
  if (!profile) {
    smsc_log_warn(rplog, "profileRespCmdHandler: can't create new profile");
    sendResponseError(cmd_ctx, profile_resp.key);
    return;
  }
  string log_msg("not registered");
  if (profile_resp.getProfile()) {
    profile_resp.getProfile()->copyPropertiesTo(profile);
    log_msg = "registered";
  }
  smsc_log_info(info_log, "profile key=%s response=OK, set state=LOCKED, profile was %s",
                 profile_resp.key.c_str(), log_msg.c_str());
  //smsc_log_debug(rplog, "profileRespCmdHandler: receive PROFILE_RESP=OK, profile was %s, set profile key=%s locked",
       //          log_msg.c_str(), profile_resp.key.c_str());
  profile->setLocked();
  getAbonentStore()->storeProfile(addr, profile);
  DoneCmd done(1, profile_resp.key);
  sendCommandToCP(done);
}

void RegionPersServer::doneCmdHandler(ConnectionContext &ctx) {
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  DoneCmd done(isb);
  AbntAddr addr(done.key.c_str());
  commands.Delete(addr);
  Profile *pf = getProfile(done.key);
  smsc_log_debug(info_log, "profile key=%s done received from CP", done.key.c_str());
  if (!pf) {
    smsc_log_warn(rplog, "doneCmdHandler: profile key=%s not found", done.key.c_str());
    return;
  }
  if (done.is_ok) {
    smsc_log_info(info_log, "profile key=%s set state=DELETED", done.key.c_str());
    //smsc_log_debug(rplog, "doneCmdHandler: receive DONE=OK profile key=%s, set state DELETED", done.key.c_str());
    pf->setDeleted();
  } else {
    smsc_log_info(info_log, "profile key=%s set state=LOCKED", done.key.c_str());
    //smsc_log_debug(rplog, "doneCmdHandler: receive DONE=ERROR profile key=%s, set state OK",
              //      done.key.c_str());
    pf->setOk();
  }
  getAbonentStore()->storeProfile(addr, pf);
}

void RegionPersServer::execClientRequest(CmdContext* cmd_ctx, Profile *profile, const std::string& pf_key) {
  AbntAddr addr(pf_key.c_str());
  if (listener.exists(cmd_ctx->socket)) {
    if (cmd_ctx->transact_batch) {
      setNeedBackup(true);
    }
    PersServerResponseType result = execCommand(cmd_ctx->cmd_id, profile, pf_key, cmd_ctx->isb, *cmd_ctx->osb, true);
    //if (result == RESPONSE_PROPERTY_NOT_FOUND || cmd_ctx->cmd_id == PC_GET || cmd_ctx->cmd_id == PC_SET) {
      //getAbonentStore()->storeProfile(addr, profile);
    //}
    Socket* socket = cmd_ctx->socket;
    commands.Delete(addr);
    sendResponseToClient(socket, result);
  } else {
    smsc_log_warn(rplog, "client connection error profile key=%s, socket %p disconnected",
                    pf_key.c_str(), cmd_ctx->socket);
    getAbonentStore()->storeProfile(addr, profile);
  }
}

void RegionPersServer::doneRespCmdHandler(ConnectionContext &ctx) {
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  DoneRespCmd done_resp(isb);
  AbntAddr addr(done_resp.key.c_str());
  smsc_log_debug(info_log, "profile key=%s done response received from CP", done_resp.key.c_str());
  CmdContext* cmd_ctx = 0;
  if (!(cmd_ctx = commands.GetPtr(addr))) {
    smsc_log_warn(rplog, "doneRespCmdHandler: command conntext key=%s not found", done_resp.key.c_str());
    return;
  }
  Profile* profile = getProfile(done_resp.key);
  if (!profile) {
    smsc_log_warn(rplog, "doneRespCmdHandler: profile key=%s not found", done_resp.key.c_str());
    sendResponseError(cmd_ctx, done_resp.key, false);
    return;
  }
  if (done_resp.is_ok) {
    //smsc_log_debug(rplog, "doneRespCmdHandler: receive DONE_RESP=OK profile key=%s, set state OK", done_resp.key.c_str());
    smsc_log_info(info_log, "profile key=%s set state=OK", done_resp.key.c_str());
    profile->setOk();
    execClientRequest(cmd_ctx, profile, done_resp.key);
    commands.Delete(addr);
  } else {
    smsc_log_warn(rplog, "doneRespCmdHandler: receive DONE_RESP=ERROR profile key=%s, set state DELETED",
                   done_resp.key.c_str());
    smsc_log_info(info_log, "profile key=%s set state=DELETED", done_resp.key.c_str());
    profile->setDeleted();
    getAbonentStore()->storeProfile(addr, profile);
    sendResponseError(cmd_ctx, done_resp.key, false);
    //sendResponseToClient(cmd_ctx->socket, RESPONSE_ERROR);
  }
}

void RegionPersServer::checkOwnRespCmdHandler(ConnectionContext &ctx) {
  SerialBuffer &osb = ctx.outbuf, &isb = ctx.inbuf;
  CheckOwnRespCmd check_own_resp(isb);
  smsc_log_info(info_log, "profile key=%s check owner response received from CP", check_own_resp.key.c_str());
  Profile *pf = getProfile(check_own_resp.key);
  if (!pf) {
    smsc_log_warn(rplog, "checkOwnRespCmdHandler: profile key=%s not found", check_own_resp.key.c_str());
    return;
  }
  AbntAddr addr(check_own_resp.key.c_str());
  CmdContext* cmd_ctx = commands.GetPtr(addr);
  switch (check_own_resp.result) {
  case ownership::OWNER:
  {
    smsc_log_debug(rplog, "checkOwnRespCmdHandler: RP is owner of profile key=%s", check_own_resp.key.c_str());
    smsc_log_info(info_log, "profile key=%s set state=OK", check_own_resp.key.c_str());
    pf->setOk();
    if (cmd_ctx) {
      execClientRequest(cmd_ctx, pf, check_own_resp.key);
    } else {
      getAbonentStore()->storeProfile(addr, pf);
    }
    break;
  }
  case ownership::NOT_OWNER: 
    smsc_log_debug(rplog, "checkOwnRespCmdHandler: RP is not owner of profile key=%s", check_own_resp.key.c_str());
    smsc_log_info(info_log, "profile key=%s set state=DELETED", check_own_resp.key.c_str());
    pf->setDeleted();
    getAbonentStore()->storeProfile(addr, pf);
    sendResponseError(cmd_ctx, check_own_resp.key, false);
    break;
  case ownership::UNKNOWN:
    smsc_log_debug(rplog, "checkOwnRespCmdHandler: profile key=%s not registered", check_own_resp.key.c_str());
    smsc_log_info(info_log, "profile key=%s set state=DELETED", check_own_resp.key.c_str());
    pf->setDeleted();
    getAbonentStore()->storeProfile(addr, pf);
    sendResponseError(cmd_ctx, check_own_resp.key, false);
    break;
  default:
    smsc_log_warn(rplog, "checkOwnRespCmdHandler: profile key=%s unknown result", check_own_resp.key.c_str());
  }
  commands.Delete(addr);
}

PersServerResponseType RegionPersServer::DelCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb) {
  IntProfileStore *is;
  bool exists = false;
  if(is = findStore(pt)) {
    smsc_log_debug(rplog, "DelCmdHandler store=%d, key=%d, name=%s", pt, int_key, name.c_str());
    exists = is->delProperty(int_key, name.c_str());
  }
  if (exists) {
    SendResponse(osb, RESPONSE_OK);
    return RESPONSE_OK;
  } else {
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
    return RESPONSE_PROPERTY_NOT_FOUND;
  }
}

PersServerResponseType RegionPersServer::GetCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb) {
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
    return RESPONSE_OK;
  } else {
    smsc_log_debug(rplog, "GetCmdHandler property not found: store=%d, key=%d, name=%s", pt, int_key, name.c_str());    
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
    return RESPONSE_PROPERTY_NOT_FOUND;
  }        
}

PersServerResponseType RegionPersServer::SetCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb) {
  IntProfileStore *is;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "SetCmdHandler store=%d, key=%d, prop=%s", pt, int_key, prop.toString().c_str());
    is->setProperty(int_key, prop);
  }
  SendResponse(osb, RESPONSE_OK);
  return RESPONSE_OK;
}

PersServerResponseType RegionPersServer::IncCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb) {
  IntProfileStore *is;
  bool exists = false;
  int result = 0;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName());
    exists = is->incProperty(int_key, prop, result);
  }
  if (exists) {
    SendResponse(osb, RESPONSE_OK);
    return RESPONSE_OK;
  } else {
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
    return RESPONSE_PROPERTY_NOT_FOUND;
  }
}

PersServerResponseType RegionPersServer::IncResultCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb) {
  IntProfileStore *is;
  bool exists = false;
  int result = 0;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "IncCmdHandler store=%d, key=%d, name=%s", pt, int_key, prop.getName());
    exists = is->incProperty(int_key, prop, result);
  }
  if(exists) {
    SendResponse(osb, RESPONSE_OK);
    osb.WriteInt32(result);
    return RESPONSE_OK;
  } else {
    SendResponse(osb, RESPONSE_TYPE_INCONSISTENCE);
    return RESPONSE_TYPE_INCONSISTENCE;
  }
}

PersServerResponseType RegionPersServer::IncModCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, int mod, SerialBuffer& osb) {
  smsc_log_debug(rplog, "IncModCmdHandler");
  IntProfileStore *is;
  bool exists = false;
  int res = 0;
  if(is = findStore(pt)) {
    smsc_log_debug(plog, "IncModCmdHandler store=%d, key=%d, name=%s, mod=%d", pt, int_key, prop.getName(), mod);
    exists = is->incModProperty(int_key, prop, mod, res);
  }
  if(exists) {
    SendResponse(osb, RESPONSE_OK);
    osb.WriteInt32(res);
    return RESPONSE_OK;
  } else {
    SendResponse(osb, RESPONSE_TYPE_INCONSISTENCE);
  }
}

PersServerResponseType RegionPersServer::DelCmdHandler(Profile *pf, const string& str_key,
                                      const string& name, SerialBuffer& osb, bool alwaysStore) {
  smsc_log_debug(rplog, "DelCmdHandler AbonetStore: key=%s, name=%s", str_key.c_str(), name.c_str());
  if (getAbonentStore()->delProperty(pf, str_key.c_str(), name.c_str())) {
    SendResponse(osb, RESPONSE_OK);
    return RESPONSE_OK;
  } else {
    if (alwaysStore) {
      AbntAddr addr(str_key.c_str());
      getAbonentStore()->storeProfile(addr, pf);
    }
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
    return RESPONSE_PROPERTY_NOT_FOUND;
  }
}

PersServerResponseType RegionPersServer::GetCmdHandler(Profile *pf, const string& str_key,
                                const string& name, SerialBuffer& osb, bool alwaysStore) {
  Property prop;
  smsc_log_debug(rplog, "GetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), name.c_str());
  if(getAbonentStore()->getProperty(pf, str_key.c_str(), name.c_str(), prop)) {   
    smsc_log_debug(rplog, "GetCmdHandler prop=%s", prop.toString().c_str());
    SendResponse(osb, RESPONSE_OK);
    prop.Serialize(osb);
    if (prop.getTimePolicy() != R_ACCESS && alwaysStore) {
      AbntAddr addr(str_key.c_str());
      getAbonentStore()->storeProfile(addr, pf);
    }
    return  RESPONSE_OK;
  } else {
    smsc_log_debug(rplog, "GetCmdHandler property not found: AbonentStore key=%s, name=%s", str_key.c_str(), name.c_str());    
    SendResponse(osb, RESPONSE_PROPERTY_NOT_FOUND);
    if (alwaysStore) {
      AbntAddr addr(str_key.c_str());
      getAbonentStore()->storeProfile(addr, pf);
    }
    return RESPONSE_PROPERTY_NOT_FOUND;
  }        
}

PersServerResponseType RegionPersServer::SetCmdHandler(Profile *pf, const string& str_key,
                                Property& prop, SerialBuffer& osb, bool alwaysStore) {
  smsc_log_debug(rplog, "SetCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.toString().c_str());
  bool result = getAbonentStore()->setProperty(pf, str_key.c_str(), prop);
  if (!result && alwaysStore) {
    AbntAddr addr(str_key.c_str());
    getAbonentStore()->storeProfile(addr, pf);
  }
  SendResponse(osb, RESPONSE_OK);
  return RESPONSE_OK;
}

PersServerResponseType RegionPersServer::IncCmdHandler(Profile *pf, const string& str_key,
                                Property& prop, SerialBuffer& osb, bool alwaysStore) {
  smsc_log_debug(rplog, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName());
  int result = 0;
  if (getAbonentStore()->incProperty(pf, str_key.c_str(), prop, result)) {
    SendResponse(osb, RESPONSE_OK);
    return RESPONSE_OK;
  } else {
    if (alwaysStore) {
      AbntAddr addr(str_key.c_str());
      getAbonentStore()->storeProfile(addr, pf);
    }
    SendResponse(osb, RESPONSE_TYPE_INCONSISTENCE);
    return RESPONSE_TYPE_INCONSISTENCE;
  }
}

PersServerResponseType RegionPersServer::IncResultCmdHandler(Profile *pf, const string& str_key,
                                Property& prop, SerialBuffer& osb, bool alwaysStore) {
  smsc_log_debug(rplog, "IncCmdHandler AbonentStore: key=%s, name=%s", str_key.c_str(), prop.getName());
  int result = 0;
  if (getAbonentStore()->incProperty(pf, str_key.c_str(), prop, result)) {
    SendResponse(osb, RESPONSE_OK);
    osb.WriteInt32(result);
    return RESPONSE_OK;
  } else {
    if (alwaysStore) {
      AbntAddr addr(str_key.c_str());
      getAbonentStore()->storeProfile(addr, pf);
    }
    SendResponse(osb, RESPONSE_TYPE_INCONSISTENCE);
    return RESPONSE_TYPE_INCONSISTENCE;
  }
}

PersServerResponseType RegionPersServer::IncModCmdHandler(Profile *pf, const string& str_key,
                                         Property& prop, int mod, SerialBuffer& osb, bool alwaysStore) {
  int res = 0;
  smsc_log_debug(rplog, "IncModCmdHandler AbonentStore: key=%s, name=%s, mod=%d", str_key.c_str(), prop.getName(), mod);
  if(getAbonentStore()->incModProperty(pf, str_key.c_str(), prop, mod, res)) {
    SendResponse(osb, RESPONSE_OK);
    osb.WriteInt32(res);
    return RESPONSE_OK;
  } else {
    if (alwaysStore) {
      AbntAddr addr(str_key.c_str());
      getAbonentStore()->storeProfile(addr, pf);
    }
    SendResponse(osb, RESPONSE_TYPE_INCONSISTENCE);
    return RESPONSE_TYPE_INCONSISTENCE;
  }
}

void RegionPersServer::createResponseForClient(SerialBuffer* sb, PersServerResponseType r,  const std::string& key) {
  if (!sb) {
    smsc_log_warn(rplog, "createResponseForClient: serial buffer error");
    return;
  }
  if (key.size()) {
    smsc_log_info(info_log, "profile key=%s send response: '%s'", key.c_str(), RESPONSE_TEXT[r]);
  }
  smsc_log_debug(rplog, "createResponseForClient: response=%d", static_cast<int>(r));
  //sb->SetPos(4);
  SendResponse(*sb, r);
  //SetPacketSize(*sb);
}

void RegionPersServer::sendResponseError(CmdContext* cmd_ctx, const string& key,
                                          bool send_done, PersServerResponseType response) {
  if (send_done) {
    DoneCmd done(0, key);
    sendCommandToCP(done);
  }
  if (!cmd_ctx) {
    return;
  }
  Socket* socket = cmd_ctx->socket;
  SerialBuffer* osb = cmd_ctx->osb;
  AbntAddr addr(key.c_str());
  commands.Delete(addr);
  if (!socket) {
    smsc_log_error(rplog, "sendResponseError: profile key=%s, socket socket error",
                    key.c_str());
    return;
  }
  if (listener.exists(socket)) {
    createResponseForClient(osb, response, key);
    sendResponseToClient(socket, response);
  } else {
    smsc_log_error(rplog, "sendResponseError: profile key=%s, socket %p disconnected",
                    key.c_str(), socket);
  }
}

void RegionPersServer::sendPacketToClient(SerialBuffer& packet, Socket* socket) {
  if (!socket) {
    return;
  }
  SetPacketSize(packet);
  packet.SetPos(0);
  listener.addRW(socket);
}

void RegionPersServer::continueExecBatch(ConnectionContext* ctx, Socket* socket) {
  if (!socket || !ctx) {
    return;
  }
  --ctx->batch_cmd_count;
  while (ctx->batch_cmd_count) {
    smsc_log_debug(rplog, "continueExecBatch: batch commands count=%d", ctx->batch_cmd_count);
    PersServerResponseType result = execCommand(*ctx);
    if (result == COMMAND_IN_PROCESS) {
      smsc_log_debug(rplog, "continueExecBatch: Batch command in process");
      setNeedBackup(false);
      return;
    } else if (ctx->transact_batch && result != RESPONSE_OK) {
      rollbackCommands(result);
      ctx->cancelBatch();
      sendPacketToClient(ctx->outbuf, socket);
      return;
    }
  }
  if (ctx->transact_batch) {
    resetStroragesBackup();
  }
  ctx->cancelBatch();
  smsc_log_debug(rplog, "continueExecBatch: send batch");
  sendPacketToClient(ctx->outbuf, socket);
}

//void RegionPersServer::sendResponseToClient(CmdContext* cmd_ctx, PersServerResponseType response) {
void RegionPersServer::sendResponseToClient(Socket* socket, PersServerResponseType response) {
  //if (!cmd_ctx || !cmd_ctx->osb) {
    //smsc_log_warn(rplog, "sendCommandToClient: serial buffer error in ctx=%p", &cmd_ctx);
    //return;
  //}
  if (!socket || !listener.exists(socket)) {
    smsc_log_warn(rplog, "sendCommandToClient: client connection error socket %p desconnectd",
                  socket);
    return;
  }
  ConnectionContext* ctx = (ConnectionContext*)socket->getData(0);
  if (!ctx) {
    smsc_log_warn(rplog, "sendCommandToClient: empty connection context");
    return;
  }
  if (ctx->transact_batch && response != RESPONSE_OK) {
    smsc_log_warn(rplog, "sendCommandToClient: abort transact batch ctx %p socket %p, error code=%d",
                  ctx, socket, response);
    rollbackCommands(response);
    ctx->cancelBatch();
    sendPacketToClient(ctx->outbuf, socket);
    return;
  }
  smsc_log_debug(rplog, "sendCommandToClient: outbuf length=%d", ctx->outbuf.GetSize());
  if (!ctx->batch || !ctx->batch_cmd_count) {
    return sendPacketToClient(ctx->outbuf, socket);
  }
  try {
    return continueExecBatch(ctx, socket);
  } catch(const SerialBufferOutOfBounds &e) {
    smsc_log_warn(rplog, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s, pos=%d",
                   ctx->inbuf.length(), ctx->inbuf.toString().c_str(), ctx->inbuf.getPos());
  }
  catch(const std::runtime_error& e) {
    smsc_log_warn(rplog, "Error profile key: %s", e.what());
  }
  catch(const Exception& e) {
    smsc_log_warn(rplog, "Exception: \'%s\'. Bad data in buffer received len=%d, data=%s",
                   e.what(), ctx->inbuf.length(), ctx->inbuf.toString().c_str());
  }
  catch(...) {
    smsc_log_warn(rplog, "Bad data in buffer received len=%d, data=%s",
                   ctx->inbuf.length(), ctx->inbuf.toString().c_str());
  }
  if (ctx->transact_batch) {
    rollbackCommands(RESPONSE_BAD_REQUEST);
    ctx->cancelBatch();
    SendResponse(ctx->outbuf, RESPONSE_BAD_REQUEST); 
    sendPacketToClient(ctx->outbuf, socket);
  }
}

void RegionPersServer::commandTimeout(const AbntAddr& addr, CmdContext* ctx) {
  if (!ctx) {
    return;
  }
  string key(addr.toString());
  CheckOwnCmd check_own(key);
  switch (ctx->wait_cmd_id) {
  case CentralPersCmd::PROFILE_RESP:
    smsc_log_warn(rplog, "PROFILE_RESP timeout profile key=%s", key.c_str());
    sendResponseError(ctx, key);
    break;
  case CentralPersCmd::DONE:
    smsc_log_warn(rplog, "DONE timeout profile key=%s", key.c_str());
    ctx->start_time = time(NULL);
    sendCommandToCP(check_own);
    break;
  case CentralPersCmd::DONE_RESP:
    smsc_log_warn(rplog, "DONE_RESP timeout profile key=%s", key.c_str());
    ctx->start_time = time(NULL);
    sendCommandToCP(check_own);
    break;
  default:
    smsc_log_warn(rplog, "UNKNWON command timeout profile key=%s", key.c_str());
  }
}

void RegionPersServer::checkTimeouts(time_t curTime)
{
	time_t time_bound = curTime - timeout;
	int i = 1;
	smsc_log_debug(rplog, "Checking timeouts...");
	while(i < listener.count())
	{
		Socket* s = listener.get(i);
		ConnectionContext* ctx = (ConnectionContext*)s->getData(0);
		if(ctx && ctx->lastActivity  < time_bound)
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
		else {
          i++;
        }
	}
    if (isStopping) {
      return;
    }
    if (!connected) {
      bindToCP(); 
    }
}

void RegionPersServer::checkTransactionsTimeouts(time_t curTime) {
  if (isStopped()) {
    return;
  }
  smsc_log_debug(rplog, "Checking transactions timeouts...");
  time_t time_bound = curTime - transactTimeout;
  AbntAddr key;
  CmdContext *ctx = 0;
  commands.First();
  while (commands.Next(key, ctx)) {
    if (ctx && ctx->start_time < time_bound) {
      commandTimeout(key, ctx);
    }
  }
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

void RegionPersServer::checkProfilesStates() {
  smsc_log_info(rplog, "Checking profiles states...");
  StringProfileStore* store = getAbonentStore();
  AbntAddr addr;
  uint16_t state_cnt = 0;
  store->Reset();
  while (store->Next(addr, state_cnt))  {
    ProfileState pf_state = static_cast<ProfileState>(state_cnt >> PROPERTIES_COUNT_SIZE);
    string pf_key(addr.toString());
    switch (pf_state) {
    case OK:
      smsc_log_debug(rplog, "profile key=%s state=OK", pf_key.c_str());
      break;
    case LOCKED: {
      smsc_log_warn(rplog, "profile key=%s state=LOCKED", pf_key.c_str());
      CheckOwnCmd check_own(pf_key);
      sendCommandToCP(check_own);
      break;
    }
    case DELETED:
      smsc_log_debug(rplog, "profile key=%s state=DELETED", pf_key.c_str());
      break;
    default:
      smsc_log_warn(rplog, "profile key=%s unknown state", pf_key.c_str());
    }
  }
}

CmdContext::CmdContext():cmd_id(PC_UNKNOWN), osb(0), socket(0), transact_batch(false),
                         start_time(time(NULL)), wait_cmd_id(CentralPersCmd::DONE) {};

CmdContext::CmdContext(PersCmd _cmd_id, const SerialBuffer& _isb, SerialBuffer* _osb, Socket *s, bool _transact_batch) :
                       cmd_id(_cmd_id), osb(_osb), socket(s), transact_batch(_transact_batch),
                       start_time(time(NULL)), wait_cmd_id(CentralPersCmd::PROFILE_RESP) {
  if (_isb.GetSize() > 0) {
    isb.blkwrite(_isb.c_curPtr(), _isb.length() - _isb.getPos());
    isb.setPos(0);
  }
}

CmdContext::CmdContext(const CmdContext& ctx):cmd_id(ctx.cmd_id), osb(ctx.osb), socket(ctx.socket),
                                              start_time(ctx.start_time), wait_cmd_id(ctx.wait_cmd_id),
                                              transact_batch(ctx.transact_batch) {
  if (ctx.isb.GetSize() > 0) {
    isb.blkwrite(ctx.isb.c_curPtr(), ctx.isb.length() - ctx.isb.getPos());
    isb.setPos(0);
  }
}

CmdContext& CmdContext::operator=(const CmdContext& ctx) {
  if (this == &ctx) {
    return *this;
  }
  isb.Empty();
  if (ctx.isb.GetSize() > 0) {
    isb.blkwrite(ctx.isb.c_curPtr(), ctx.isb.length() - ctx.isb.getPos());
    isb.setPos(0);
  }
  cmd_id = ctx.cmd_id;
  osb = ctx.osb;
  socket = ctx.socket;
  start_time = ctx.start_time;
  wait_cmd_id = ctx.wait_cmd_id;
  transact_batch = ctx.transact_batch;
  //batch = ctx.batch;
  return *this;
}


}//pers
}//scag

