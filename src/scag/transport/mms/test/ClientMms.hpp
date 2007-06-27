#ifndef __SCAG_TRANSPORT_MMS_CLIENTMMS_HPP__
#define __SCAG_TRANSPORT_MMS_CLIENTMMS_HPP__

/*
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"
#include <string>
#include <cstring>
#include <string.h>
#include <memory>
*/

#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"
#include "scag/transport/mms/MmsCommand.h"
#include "scag/transport/mms/MmsParser.hpp"


namespace scag { 
namespace transport { 
namespace mms {
namespace test {

using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::logger::Logger;

static const char* LOCAL_HOST = "192.168.1.12";//"127.0.0.1";
static const char* BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
static const char* OK_RESPONSE = "HTTP/1.1 200 Ok\r\n";
static const char* SAMPLE_POST = "POST /mms-rs/mm7 HTTP/1.1\r\n";
static const size_t SAMPLE_POST_SIZE = std::strlen(SAMPLE_POST);
static const size_t OK_RESPONSE_SIZE = std::strlen(OK_RESPONSE);
static const int BAD_REQUEST_SIZE = std::strlen(BAD_REQUEST);
static const char* SERVER_ERROR = "HTTP/1.1 500 Internal Server Error\r\n";
static const int SERVER_ERROR_SIZE = std::strlen(SERVER_ERROR);
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
static size_t TID_TEMPLATE_SIZE = std::strlen(TID_TEMPLATE);

static Mutex tid_mutex;

using smsc::core::threads::Thread;

class ClientMms {
public:
  ClientMms(string _host, int _port, bool _is_vasp = true):host(_host), port(_port), is_vasp(_is_vasp), logger(0) {
    logger = Logger::getInstance("mms.client");
    setPrefix();
  }

  virtual ~ClientMms() {
    smsc_log_debug(logger, "Delete Client");
  }

  bool isVASP() {
    return is_vasp;
  }

  virtual bool start() {
    smsc_log_debug(logger, "Start MMS Client");
    smsc_log_debug(logger, "MMS Relay Server \'%s:%d\'", host.c_str(), port);
    if (socket.Init(host.c_str(),port, CLIENT_TIME_OUT) < 0) {
      smsc_log_error(logger, "Can't Init Socket on host = \'%s\' port = %d", host.c_str(), port);
      return false;
    }
    return true;
  }

  bool sendCommand(string cmd_template, string soap_attchment) {
    MmsCommand cmd;
    //__trace2__("SOAP ENVELOPE SIZE = %d", cmd_template.size());
    if (!cmd.createMM7Command(cmd_template.c_str(), cmd_template.size())) {
      return false;
    }
    cmd.setTransactionId(getTransactionId(prefix));
    return sendVASPRequest(cmd, soap_attchment);
  }

  bool flood(string cmd_template, string soap_attachment, const Hash<string>& fields, 
             vector<string> values, string template_string = ADR_TEMPLATE) {
    int n = values.size();
    if (n == 0) {
      return false;
    }
    HttpPacket packet;
    if (soap_attachment.empty()) {
      packet = createRequestPacket();
    } else {
      packet = createMultipartRequestPacket();
      packet.setSoapAttachment(soap_attachment.c_str(), soap_attachment.size());
    }
    size_t templ_size = template_string.size();
    size_t pos = cmd_template.find(template_string);
    size_t tid_pos = cmd_template.find(TID_TEMPLATE);
    for (vector<string>::iterator i = values.begin(); i != values.end(); ++i) {
      string cmd(cmd_template);
      if (pos != string::npos) {
        cmd.replace(pos, templ_size, *i);
      }
      if (tid_pos != string::npos) {
        string tid = getTransactionId(prefix);
        cmd.replace(tid_pos, TID_TEMPLATE_SIZE, tid);
      }
      if (!start()) {
        printf("\nCan't Init Client Socket\n");
        return false;
      }
      packet.setSoapEnvelope(cmd);
      if (sendRequestPacket(packet.serialize().c_str())) {
        printf("Success Send\n");
      } else {
        printf("Error Send\n");
      }
    }
    return true;
  }
  bool sendCommand(string cmd_template, string soap_attchment, const Hash<string>& fields) {
    MmsCommand cmd;
    //__trace2__("SOAP ENVELOPE SIZE = %d", cmd_template.size());
    if (!cmd.createMM7Command(cmd_template.c_str(), cmd_template.size())) {
      return false;
    }
    cmd.setTransactionId(getTransactionId(prefix));
    Hash<string>::Iterator i = fields.getIterator();
    i.First();
    char* name = 0;
    string value;
    while(i.Next(name, value)) {
      cmd.setInfoElement(name, value);
    }
    return sendVASPRequest(cmd, soap_attchment);
  }

  bool sendRequestPacket(const char* packet) {
    //__trace__("SEND REQUEST PACKET!!!\n");
    size_t packet_size = 0;
    if (!packet || !(packet_size = std::strlen(packet))) {
      smsc_log_error(logger, "Request Packet is Empty");
      return false;
    }
    if (socket.Connect() < 0) {
      smsc_log_error(logger, "Can't Connect to Server Host = \'%s\' Port = %d", host.c_str(), port);
      return false;
    }
    incTransactionId();
    //int n = socket.Write(packet, 1024);
    int n = socket.Write(packet, packet_size);
    //int n = socket.Write("", 0);
    smsc_log_debug(logger, "Request Packet Size = %d", packet_size);
    smsc_log_debug(logger, "Client Sent %d Bytes", n);
    smsc_log_debug(logger, "Request Packet:\n\n\'%s\'\n", packet);
    smsc_log_debug(logger, "Start Receive Response Packet");
    //socket.setTimeOut(10);
    std::string in_packet = recvPacket(&socket, SERVER_TIME_OUT * 10);
    smsc_log_debug(logger, "Response Packet:\n\n\'%s\'\n", in_packet.c_str());
    /********************/
    socket.Close();
    /********************/
    //HttpMsg http_msg;
    //if (!MmsParser::parse(in_packet.c_str(), in_packet.size(), http_msg, logger)) {
    HttpPacket http_packet;
    if (!http_packet.parse(in_packet.c_str(), in_packet.size())) {
      smsc_log_error(logger, "Error While Parsing Response Packet");
      return false;
    }
    if (http_packet.isErrorResp()) {
      smsc_log_debug(logger, "Client Recive Error Resp");
      return true;
    }
    MmsCommand in_cmd;
    //const char* soap_env = http_packet.getSoapEnvelope();
    string soap_env = http_packet.getSoapEnvelope();
    smsc_log_debug(logger, "Create Response MM7 Command");
    if (in_cmd.createMM7Command(soap_env.c_str(), soap_env.size())) {
      smsc_log_debug(logger, "Serialized Response Command:\n\n\'%s\'\n", in_cmd.serialize().c_str());
    } else {
      smsc_log_error(logger, "Can't Create Response MM7 Command");
    }
    return true;
  }
  void closeSocket() {
    socket.Close();
  }

protected:
  inline std::string recvPacket(Socket* _socket, int time_out) {
    const size_t buf_size = 1024;
    char buf[buf_size];
    std::string packet;
    size_t packet_size = 0;
    int n = 0;
    _socket->setTimeOut(time_out);
    while (_socket->canRead() && ((n = _socket->Read(buf, buf_size)) > 0)) {
      //__trace2__("READ %d byte", n);
      packet.append(buf, n);
      packet_size += n;
      if (n < buf_size) {
        return packet;
      }
    }
    return packet;
  }

  HttpPacket createRequestPacket(size_t content_length = 0) {
    HttpHeader header;
    header.setContentType(TEXT_XML);
    header.setCharset(xml::UTF_8);
    header.addField(HOST, SAMPLE_HOST);
    header.addField(SOAP_ACTION, "\"\"");
    header.addField(CONTENT_LENGTH, SAMPLE_CONTENT_LENGTH);
    if (content_length > 0) {
      const size_t buf_size = 1024;
      char buf[buf_size];
      int n = snprintf(buf, buf_size, "%d", content_length);
      if (n > 0) {
        header.addField(CONTENT_LENGTH, CONTENT_LENGTH_SIZE, buf, n);
      }
    }
    HttpPacket packet;
    packet.setHttpHeader(header);
    packet.setStartLine(SAMPLE_POST, SAMPLE_POST_SIZE);
    return packet;
  }

  HttpPacket createResponsePacket(size_t content_length = 0) {
    HttpHeader header;
    header.setContentType(TEXT_XML);
    header.setCharset(xml::UTF_8);
    header.addField(SOAP_ACTION, "\"\"");
    header.addField(CONTENT_LENGTH, SAMPLE_CONTENT_LENGTH);
    if (content_length > 0) {
      const size_t buf_size = 14;
      char buf[buf_size];
      int n = snprintf(buf, buf_size, "%d", content_length);
      if (n > 0) {
        header.addField(CONTENT_LENGTH, CONTENT_LENGTH_SIZE, buf, n);
      }
    }
    HttpPacket packet;
    packet.setHttpHeader(header);
    packet.setStartLine(OK_RESPONSE, OK_RESPONSE_SIZE);
    return packet;
  }

  HttpPacket createMultipartRequestPacket(size_t content_length = 0, 
                                   const char* boundary = SAMPLE_BOUNDARY, const char* start = SAMPLE_START) {
    HttpHeader header;
    header.setBoundary(boundary);
    header.setContentType(MULTIPART);
    header.addContentTypeParam(START, start);
    header.addContentTypeParam(TYPE, TEXT_XML);
    header.addField(HOST, SAMPLE_HOST);
    header.addField(SOAP_ACTION, "\"\"");
    header.addField(CONTENT_LENGTH, SAMPLE_CONTENT_LENGTH);
    if (content_length > 0) {
      const size_t buf_size = 12;
      char buf[buf_size];
      int n = snprintf(buf, buf_size, "%d", content_length);
      if (n > 0) {
        header.addField(CONTENT_LENGTH, CONTENT_LENGTH_SIZE, buf, n);
      }
    }
    HttpHeader envelope_header;
    envelope_header.setContentType(TEXT_XML);
    envelope_header.setCharset(xml::UTF_8);
    envelope_header.addField(CONTENT_ID, start);
    HttpPacket packet;
    packet.setEnvelopeHeader(envelope_header);
    packet.setHttpHeader(header);
    packet.setStartLine(SAMPLE_POST, SAMPLE_POST_SIZE);
    return packet;
  }
  void setPrefix() {
    if (is_vasp) {
      prefix = "VASP_";
    } else {
      prefix = "RS_";
    }
  }
  static void incTransactionId() {
    MutexGuard mg(tid_mutex);
    ++transaction_id;
  }
  static string getTransactionId(string _prefix) {
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
  std::string host;
  bool is_vasp;
  static int transaction_id;
  string prefix;

private:
  bool sendVASPRequest(const MmsCommand& cmd, string soap_attchment) {
    string soap_envelope = cmd.serialize();
    //__trace2__("SEND VASPREQUEST ENVELOPE SIZE = %d", soap_envelope.size());
    if (soap_attchment.empty()) {
      HttpPacket packet = createRequestPacket(soap_envelope.size());
      packet.setSoapEnvelope(soap_envelope.c_str(), soap_envelope.size());
      return sendRequestPacket(packet.serialize().c_str());
    }
    HttpPacket packet = createMultipartRequestPacket();
    packet.setSoapEnvelope(soap_envelope.c_str(), soap_envelope.size());
    packet.setSoapAttachment(soap_attchment.c_str(), soap_attchment.size());
    return sendRequestPacket(packet.serialize().c_str());  
  }

};


}//test
}//mms
}//transport
}//scag

#endif
