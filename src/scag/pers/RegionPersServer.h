#ifndef __SCAG_PERS_REGIONPERSSERVER_H__
#define __SCAG_PERS_REGIONPERSSERVER_H__
 
#include "PersServer.h"
#include "core/buffers/XHash.hpp"
#include "CPersCmd.h"

namespace scag { namespace pers {

using namespace scag::cpers;

struct CmdContext {
  CmdContext();
  CmdContext(const CmdContext& ctx);
  CmdContext(PersCmd _cmd_id, const SerialBuffer& _isb, SerialBuffer* _osb, Socket *s);
  CmdContext(uint8_t _wait_cmd_id);
  CmdContext& operator=(const CmdContext& ctx);

  PersCmd cmd_id;
  uint8_t wait_cmd_id;
  SerialBuffer isb;
  SerialBuffer* osb;
  Socket* socket;     
  time_t start_time; 
  bool batch;
};

class RegionPersServer : public PersServer {
public:
  RegionPersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_,
                   int transactTimeout, StringProfileStore *abonent,IntProfileStore *service,
                   IntProfileStore *oper, IntProfileStore *provider, const string& _central_host,
                   int _central_port, uint32_t _region_id, const string& _region_psw);
  virtual bool processPacket(ConnectionContext &ctx);

protected:
  virtual void onDisconnect(ConnectionContext &ctx);
  //virtual void processUplinkPacket(ConnectionContext &ctx) {
  //}
  virtual bool bindToCP();
  void checkTimeouts();

private:
  void connectToCP();
  bool pingCP();
  bool processPacketFromCP(ConnectionContext &ctx);
  bool processPacketFromClient(ConnectionContext &ctx);
  void execCommand(ConnectionContext &ctx);
  void sendCommandToClient(CmdContext *ctx);
  void sendResponseError(CmdContext* cmd_ctx, const string& key, bool send_done = true);

  bool sendCommandToCP(const CPersCmd& cmd);
  void createResponseForClient(SerialBuffer *sb, PersServerResponseType r);

  void getProfileCmdHandler(ConnectionContext& ctx);
  void profileRespCmdHandler(ConnectionContext& ctx);
  void doneCmdHandler(ConnectionContext& ctx);
  void doneRespCmdHandler(ConnectionContext& ctx);
  void checkOwnRespCmdHandler(ConnectionContext& ctx);
  void execCommand(PersCmd cmd, ProfileType pt, uint32_t int_key, const string& str_key,
                   SerialBuffer& isb, SerialBuffer& osb);
  bool execCommand(PersCmd cmd, Profile *pf, const string& str_key,
                   SerialBuffer& isb, SerialBuffer& osb);

  void DelCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb);
  void GetCmdHandler(ProfileType pt, uint32_t int_key, const string& name, SerialBuffer& osb);
  void SetCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb);
  void IncCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, SerialBuffer& osb);
  void IncModCmdHandler(ProfileType pt, uint32_t int_key, Property& prop, int mod, SerialBuffer& osb);

  void DelCmdHandler(Profile* pf, const string& str_key, const string& name, SerialBuffer& osb);
  bool GetCmdHandler(Profile* pf, const string& str_key, const string& name, SerialBuffer& osb);
  void SetCmdHandler(Profile* pf, const string& str_key, Property& prop, SerialBuffer& osb);
  void IncCmdHandler(Profile* pf, const string& str_key, Property& prop, SerialBuffer& osb);
  void IncModCmdHandler(Profile* pf, const string& str_key, Property& prop, int mod, SerialBuffer& osb);

  void checkTransactionsTimeouts();
  void commandTimeout(const AbntAddr& addr, CmdContext* ctx);

  void WriteAllToCP(const char* buf, uint32_t sz);
  void ReadAllFromCP(char* buf, uint32_t sz);

  void checkProfilesStates();


private:
  Socket *central_socket;
  string central_host;
  int central_port;
  uint32_t region_id;
  string region_psw;
  bool connected;
  time_t last_check_time;

  smsc::core::buffers::XHash<AbntAddr, CmdContext, AbntAddr> commands;
  //smsc::core::buffers::XHash<AbntAddr, uint8_t, AbntAddr> profiles_states;
  //smsc::core::buffers::XHash<AbntAddr, time_t, AbntAddr> requested_profiles;
  Logger* rplog;
};

}//pers
}//scag
#endif 

