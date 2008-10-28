#ifndef __SCAG_MTPERS_MTPERSCLIENT_H__
#define __SCAG_MTPERS_MTPERSCLIENT_H__


#include <string>
#include "core/threads/ThreadPool.hpp"
#include "core/threads/ThreadedTask.hpp"

#include "scag/pers/util/PersClient.h"
#include "scag/util/storage/SerialBuffer.h"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"

namespace scag { namespace mtpers {

using namespace scag::pers::util;
using smsc::logger::Logger;
using std::string;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::ThreadedTask;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

class MTPersClient {

public:
  MTPersClient(const string& _host, int _port, int _timeout, int pingTimeout);
  ~MTPersClient();
  void SetProperty(ProfileType pt, const PersKey& key, Property& prop, bool batch = false);
  void GetProperty(ProfileType pt, const PersKey& key, const char *property_name, Property& prop, bool batch = false);
  bool DelProperty(ProfileType pt, const PersKey& key, const char *property_name, bool batch = false);
  int IncProperty(ProfileType pt, const PersKey& key, Property& prop, bool batch = false);
  int IncModProperty(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod, bool batch = false);

  void SetPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb, bool batch = false);
  void GetPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb, bool batch = false);
  void DelPropertyPrepare(ProfileType pt, const PersKey& key, const char *property_name, SerialBuffer& bsb, bool batch = false);
  void IncPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, SerialBuffer& bsb, bool batch = false);
  void IncModPropertyPrepare(ProfileType pt, const PersKey& key, Property& prop, uint32_t mod, SerialBuffer& bsb, bool batch = false);

  void SetPropertyResult(SerialBuffer& bsb);
  void GetPropertyResult(Property& prop, SerialBuffer& bsb);
  bool DelPropertyResult(SerialBuffer& bsb);
  int IncPropertyResult(SerialBuffer& bsb);
  int IncModPropertyResult(SerialBuffer& bsb);

  void PrepareBatch(SerialBuffer& bsb, ProfileType pt, const PersKey& key, uint16_t cnt, bool transactMode = false);
  void RunBatch(SerialBuffer& sb);
  void FinishPrepareBatch(uint32_t cnt, SerialBuffer& bsb);

  void init();
  void disconnect();
  void testCase(const string& address, int key);
  void testCase_CommandProcessing(ProfileType pt, const char* address, int key);
  void testCase_DelProfile(ProfileType pt, const char* address, int intKey);

private:
  void setBatchCount(uint32_t cnt, SerialBuffer& bsb);    
  void reinit(const char *_host, int _port, int _timeout, int _pingTimeout);
  void setPacketSize(SerialBuffer& bsb);

  void emptyPacket(SerialBuffer& bsb) { bsb.Empty(); bsb.SetPos(4); }

  void FillHead(PersCmd pc, ProfileType pt, const PersKey& key, SerialBuffer& bsb, bool batch = false);
  void SendPacket(SerialBuffer& bsb);
  void ReadPacket(SerialBuffer& bsb);
  void WriteAllTO(const char* buf, uint32_t sz);
  void ReadAllTO(char* buf, uint32_t sz);
  uint32_t getPacketSize(SerialBuffer& bsb);
  PersServerResponseType GetServerResponse(SerialBuffer& bsb);
  void ping();
  void CheckServerResponse(SerialBuffer& bsb);

private:
  string host;
  int port;
  int timeout;
  int pingTimeout;
  time_t actTS;
  Socket sock;
  bool connected;
  Logger * log;
  SerialBuffer sb;
};

class ClientTask: public ThreadedTask {
public:
  ClientTask(const string& host, int port, int index):client(host, port, 60, 5), taskIndex(index) { 
    logger = Logger::getInstance("task");
    smsc_log_error(logger, "ClientTask created %p", this);
  };
  ~ClientTask() {};
  virtual int Execute();
  virtual const char * taskName() {
    return "ClientTask";
  }
  virtual void stop() {
    isStopping = false;
  }

private:
  MTPersClient client;
  Logger* logger;
  int taskIndex;
};

}//mtpers
}//scag

#endif

