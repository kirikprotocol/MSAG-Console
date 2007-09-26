#ifndef __SCAG_TRANSPORT_MMS_CLIENTMMS_H__
#define __SCAG_TRANSPORT_MMS_CLIENTMMS_H__

#include <time.h>
#include <unistd.h>

#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"
#include "scag/transport/mms/MmsCommand.h"
#include "scag/transport/mms/MmsParser.hpp"


namespace scag { namespace transport { namespace mms { namespace test {

using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::logger::Logger;

static const char* LOCAL_HOST = "192.168.1.12";//"127.0.0.1";
static const char* BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
static const char* SAMPLE_POST = "POST /mms-rs/mm7 HTTP/1.1\r\n";
static const size_t SAMPLE_POST_SIZE = strlen(SAMPLE_POST);
static const size_t OK_RESPONSE_SIZE = strlen(OK_RESPONSE);
static const int BAD_REQUEST_SIZE = strlen(BAD_REQUEST);
static const char* SERVER_ERROR = "HTTP/1.1 500 Internal Server Error\r\n";
static const int SERVER_ERROR_SIZE = strlen(SERVER_ERROR);
static const int SERVER_TIME_OUT = 10;
static const size_t CLIENT_TIME_OUT = 100;

static const char* SAMPLE_BOUNDARY = "NextPart_000_0128_01C19839.12345";
static const char* SAMPLE_START = "</tnn-123456/mm7-request>";
static const char* MULTIPART_RELATED = "multipart/related";
static const char* SAMPLE_HOST = "msag";
//static const char* SAMPLE_POST = "/mms-rs/mm7"
static const char* CONTENT_ID = "Content-ID";

static const char* ADR_TEMPLATE = "@address@";
static const char* TID_TEMPLATE = "@tid@";
static const char* EPID_TEMPLATE = "@endpoint_id@";
static size_t TID_TEMPLATE_SIZE = strlen(TID_TEMPLATE);
static size_t EPID_TEMPLATE_SIZE = strlen(EPID_TEMPLATE);

static Mutex tid_mutex;

using smsc::core::threads::Thread;
using std::string;

class ClientMms {
public:
  ClientMms(const string& _host, int _port, const string& ep_id, bool _is_vasp = true)
   :host(_host), port(_port), endpoint_id(ep_id), is_vasp(_is_vasp), logger(0) {
    logger = Logger::getInstance("mms.client");
    setPrefix();
  }
     
  virtual ~ClientMms() {
    smsc_log_debug(logger, "Delete Client");
  }

  bool isVASP() {
    return is_vasp;
  }

  virtual bool start();
  bool sendCommand(const string& cmd_template, const string& soap_attchment);
  void sendRequestWithPrintf(const char* packet);
  bool flood(string& cmd_template, const string& soap_attachment, const Hash<string>& fields, 
             const vector<string>& values, const char* template_string = ADR_TEMPLATE);

  bool sendCommand(const string& cmd_template, const string& soap_attchment,
                   const Hash<string>& fields);

  bool sendRequestPacket(const char* packet);
  void closeSocket();

protected:
  void recvPacket(Socket* _socket, int time_out, string& packet);

  void createRequestPacket(HttpPacket& packet, size_t content_length = 0);

  void createResponsePacket(HttpPacket& packet, size_t content_length = 0);

  void createMultipartRequestPacket(HttpPacket& packet, size_t content_length = 0, 
                                   const char* boundary = SAMPLE_BOUNDARY,
                                   const char* start = SAMPLE_START);
  void setPrefix();
  static void incTransactionId() {
    MutexGuard mg(tid_mutex);
    ++transaction_id;
  }
  static string getTransactionId(const string& _prefix) {
    string tid(_prefix);

    const size_t buf_size = 12;
    char buf[buf_size];
    int n = 0;
    {
      MutexGuard mg(tid_mutex);
      n = snprintf(buf, buf_size, "%d", transaction_id);
    }
    size_t zero_count = buf_size - n < 0 ? 0 : buf_size - n;
    for (int i = 0; i < zero_count; ++i) {
      tid += "0";
    }
    tid.append(buf, n);
    return tid;
  }
protected:
  Socket socket;
  Logger* logger;
  int port;
  string host;
  string endpoint_id;
  bool is_vasp;
  static int transaction_id;
  string prefix;

private:
  bool sendVASPRequest(const MmsCommand& cmd, const string& soap_attchment);

};


}//test
}//mms
}//transport
}//scag

#endif
