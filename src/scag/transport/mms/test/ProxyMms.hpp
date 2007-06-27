#ifndef __SCAG_TRANSPORT_MMS_PROXYMMS_HPP
#define __SCAG_TRANSPORT_MMS_PROXYMMS_HPP

#include "ServerMms.hpp"

namespace scag {
namespace transport {
namespace mms {
namespace test {

class ProxyMms : public ServerMms {
public:

  ProxyMms(string proxy_host, int proxy_port, string _service_host, int _service_port, bool _is_vasp)
  :ServerMms(proxy_host, proxy_port, _is_vasp), service_host(_service_host), service_port(_service_port) {
    prefix = "MSAG_";
    logger = Logger::getInstance("mms.proxy");
    __trace2__("Create MMS Proxy %s:%d", host.c_str(), port);
  }

protected:

  void work(Socket* sock, size_t con_number) {
    if (sock == NULL) {
      return;
    }
    smsc_log_debug(logger, "Accept Client %d", con_number);
    smsc_log_debug(logger, "Start Receive Request Packet");

    std::string packet = recvPacket(sock, SERVER_TIME_OUT);

    smsc_log_debug(logger, "Request Packet size = %d bytes", packet.size());
    smsc_log_debug(logger, "Request Packet:\n\n\'%s\'\n", packet.c_str());
    if (packet.empty()) {
      smsc_log_error(logger, "Request Packet is Empty");

      sock->Write(BAD_REQUEST, BAD_REQUEST_SIZE);

      return;
    }
    HttpPacket http_packet;
    if (!http_packet.parse(packet.c_str(), packet.size())) {
      if (http_packet.isRequest()) {
        smsc_log_error(logger, "Error While Parsing Request Packet");
        smsc_log_debug(logger, "Response Packet Size = %d bytes", BAD_REQUEST_SIZE);
        smsc_log_debug(logger, "Response Packet:\n\n\'%s\'\n",  BAD_REQUEST);

        int n = sock->Write(BAD_REQUEST, BAD_REQUEST_SIZE);

        smsc_log_debug(logger, "Server Sent %d bytes", n);
      }
      return;
    }
    string soap_envelope = http_packet.getSoapEnvelope();
    MmsCommand in_cmd;
    if (!in_cmd.createMM7Command(soap_envelope.c_str(), soap_envelope.size())) {
      smsc_log_error(logger, "Error While Creating Request MM7 Command");
      processErrorCommand(sock, in_cmd);
      return;
    }
    smsc_log_debug(logger, "Request Command Id = %d", in_cmd.getCommandId());
    //__trace2__("Http Packet:\n\n\'%s\'\n", http_packet.serialize().c_str());
    if (is_vasp) {
      workAsVASP(sock, in_cmd, http_packet);
    } else {
      workAsRS(sock, in_cmd, http_packet);
    }
  }

private:

  void processCommand(Socket* sock, MmsCommand& in_cmd, HttpPacket& in_packet) {
    Socket client_socket;
    if (client_socket.Init(service_host.c_str(), service_port, CLIENT_TIME_OUT) < 0 ||
        client_socket.Connect() < 0) {
      sendResp(sock, in_cmd, "4006", "Service Unavailable");
      return;
    }
    string client_tid = in_cmd.getTransactionId();
    string proxy_host = in_packet.getHost();
    in_packet.setHost(service_host);
    in_cmd.setTransactionId(getTransactionId(prefix));

    incTransactionId();

    string soap_envelope = in_cmd.serialize();
    in_packet.setSoapEnvelope(soap_envelope);

    string packet = in_packet.serialize();
    if (packet.empty()) {
      in_cmd.setTransactionId(client_tid);
      sendResp(sock, in_cmd, "3000", "Server Error");
      return;
    }
    smsc_log_debug(logger, "Proxy Send Packet size = %d\n\n\'%s\'\n", packet.size(), packet.c_str());
    int wn = client_socket.Write(packet.c_str(), packet.size());
    smsc_log_debug(logger, "Proxy Sent %d bytes", wn);

    smsc_log_debug(logger, "Proxy Start Recieve Response");
    string resp_buf = recvPacket(&client_socket, SERVER_TIME_OUT * 10);
    smsc_log_debug(logger, "Response Packet:\n\n\'%s\'\n", resp_buf.c_str());
    /********************/
    client_socket.Close();
    /*******************/

    HttpPacket resp_packet;
    if (!resp_packet.parse(resp_buf.c_str(), resp_buf.size())) {
      smsc_log_error(logger, "Error While Parsing Response Packet");
      in_cmd.setTransactionId(client_tid);
      sendResp(sock, in_cmd, "4005", "Service Error");
      return;
    }
    if (resp_packet.isErrorResp()) {
      smsc_log_error(logger, "Proxy Recieve Error Response");
      in_cmd.setTransactionId(client_tid);
      sendResp(sock, in_cmd, "4010", "Bad Request");
      return;
    }
    MmsCommand resp_cmd;
    string soap_env = resp_packet.getSoapEnvelope();
    smsc_log_debug(logger, "Create Response MM7 Command");

    if (resp_cmd.createMM7Command(soap_env.c_str(), soap_env.size())) {
      //smsc_log_debug(logger, "Serialized Response Command:\n\n\'%s\'\n", in_cmd.serialize().c_str());

      resp_cmd.setTransactionId(client_tid);

      ServerMms::sendResp(sock, resp_cmd);

    } else {
      smsc_log_error(logger, "Can't Create Response MM7 Command");

      in_cmd.setTransactionId(client_tid);

      sendResp(sock, in_cmd, "4010", "Bad Request");
    }
  }

  void workAsRS(Socket* sock, MmsCommand& in_cmd, HttpPacket& in_packet) {
    __trace__("Proxy Start Work as RS");
    uint8_t cmd_id = in_cmd.getCommandId();
    if (cmd_id != MM7_SUBMIT && cmd_id != MM7_CANCEL && cmd_id != MM7_REPLACE &&
        cmd_id != MM7_EXTENDED_CANCEL && cmd_id != MM7_EXTENDED_REPLACE) {
      sendRSErrorResp(sock, in_cmd.getTransactionId(), "4003", "Unsupported Operation");
      return;
    }
    processCommand(sock, in_cmd, in_packet);
  }

  void workAsVASP(Socket* sock, MmsCommand& in_cmd, HttpPacket& in_packet) {
    __trace__("Proxy Start Work as VASP");
    uint8_t cmd_id = in_cmd.getCommandId();
    if (cmd_id != MM7_DELIVER && cmd_id != MM7_DELIVERY_REPORT && cmd_id != MM7_READ_REPLY) {
      sendVASPErrorResp(sock, in_cmd.getTransactionId(), "4003", "Unsupported Operation");
      return;
    }
    processCommand(sock, in_cmd, in_packet);
  }
  void sendRSErrorResp(Socket* sock, string tid, string status_code, string status_text) {
    MmsCommand resp_cmd;
    resp_cmd.createRSError(tid, status_code, status_text);
    ServerMms::sendResp(sock, resp_cmd);
  }

  void sendVASPErrorResp(Socket* sock, string tid, string status_code, string status_text) {
    MmsCommand resp_cmd;
    resp_cmd.createVASPError(tid, status_code, status_text);
    ServerMms::sendResp(sock, resp_cmd);
  }

  void sendResp(Socket* sock, const MmsCommand& in_cmd, string status_code, string status_text) {
    MmsCommand resp_cmd;
    resp_cmd.createResponse(in_cmd.getMmsMsg(), status_code, status_text);
    ServerMms::sendResp(sock, resp_cmd);
  }

private:

  string service_host;
  int service_port;
};

}//test
}//mms
}//transport
}//svag

#endif
