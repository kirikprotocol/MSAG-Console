#include "ServerMms.hpp"

namespace scag { namespace transport { namespace mms { namespace test {

const char* SERVER_ERROR = "HTTP/1.1 500 Internal Server Error\r\n";
const int SERVER_ERROR_SIZE = strlen(SERVER_ERROR);

const char* BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
const int BAD_REQUEST_SIZE = strlen(BAD_REQUEST);

bool ServerMms::start() {
  if (socket.InitServer(host.c_str(), port, SERVER_TIME_OUT, true) < 0) {
    smsc_log_error(logger, "Can't Init Server on Host = \'%s\' Port = %d", host.c_str(), port);
    return false;
  }
  smsc_log_debug(logger, "Init Server on Host = \'%s\' Port = %d", host.c_str(), port);
  if (socket.StartServer() < 0) {
    smsc_log_error(logger, "Can't Start Server");
    return false;
  }
  smsc_log_debug(logger, "Start Server");
  size_t con_number = 0;
  for (;;) {
    std::auto_ptr<Socket> sock(socket.Accept());
    work(sock.get(), con_number);
    ++con_number;
  }
}

//protected:
void ServerMms::work(Socket* sock, size_t con_number) {
  if (sock == NULL) {
    return;
  }
  smsc_log_debug(logger, "Accept Client %d", con_number);
  smsc_log_debug(logger, "Start Receive Request Packet");

  std::string packet;
  recvPacket(sock, SERVER_TIME_OUT, packet);

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
    smsc_log_error(logger, "Error While Parsing Response Packet");
    return;
  }
  //string soap_envelope = http_packet.getSoapEnvelope();
  MmsCommand in_cmd;
  if (!in_cmd.createMM7Command(http_packet.getSoapEnvelope(),
                               http_packet.getSoapEnvelopeSize())) {
    smsc_log_error(logger, "Error While Creating Request MM7 Command");
    processErrorCommand(sock, in_cmd);
    return;
  }
  MmsCommand out_cmd; 
  if (out_cmd.createResponse(in_cmd.getMmsMsg(), status::SUCSESS)) {
    sendResp(sock, out_cmd);
  } else {
    sock->Write(BAD_REQUEST, BAD_REQUEST_SIZE);
  }
}

void ServerMms::sendResp(Socket* sock, const MmsCommand& cmd) {
  std::string serialized_cmd;
  if (!cmd.serialize(serialized_cmd)) {
    smsc_log_error(logger, "Serilize response command ERROR");
    sock->Write(SERVER_ERROR, SERVER_ERROR_SIZE);
    return;
  }
  smsc_log_debug(logger, "serilized response command=\'%s\'", serialized_cmd.c_str());
  HttpPacket http_packet;
  createResponsePacket(http_packet, serialized_cmd.size());
  http_packet.setSoapEnvelope(serialized_cmd.c_str(), serialized_cmd.size());
  string serialized_packet;
  http_packet.serialize(serialized_packet);
  smsc_log_debug(logger, "Response Packet Size = %d bytes", serialized_packet.size());
  smsc_log_debug(logger, "Response Packet:\n\n\'%s\'\n",  serialized_packet.c_str());

  int n = sock->Write(serialized_packet.c_str(), serialized_packet.size());

  smsc_log_debug(logger, "Server Sent %d bytes", n);
}

void ServerMms::processErrorCommand(Socket* sock, const MmsCommand& command) {
  const MmsMsg* msg = command.getMmsMsg();
  if (msg) {
    MmsCommand resp_cmd;
    if (resp_cmd.createResponse(msg, status::VALIDATION_ERROR)) {
      sendResp(sock, resp_cmd);
    } else {
      sock->Write(BAD_REQUEST, BAD_REQUEST_SIZE);
    }
    return;
  }
  if (command.getTransactionId().empty()) {
    sock->Write(BAD_REQUEST, BAD_REQUEST_SIZE); 
    return;
  }
  MmsCommand resp_cmd;
  //if (is_vasp) {
    //resp_cmd.createVASPError(command.getTransactionId(), "4004", "Validation Error");
  //} else {
    //resp_cmd.createRSError(command.getTransactionId(), "4004", "Validation Error");
  //}
  resp_cmd.createGenericError(command.getTransactionId(),
                              status::VALIDATION_ERROR, is_vasp);
  sendResp(sock, resp_cmd);
}

/*
  void sendResp(Socket* sock, const MmsCommand& cmd) {
    HttpMsg http_msg;
    http_msg.setFirstLine("HTTP/1.1 200 Ok\r\n");
    http_msg.setHeader("Content-Type: text/xml; charset=\'utf-8\'\r\nContent-Length: nnnn\r\n\r\n");
    std::string serialized_cmd = cmd.serialize();      
    if (serialized_cmd.empty()) {
      sock->Write(SERVER_ERROR, SERVER_ERROR_SIZE);
      return;
    }
    http_msg.setSoapEnvelope(serialized_cmd.c_str());
    std::string http_packet = http_msg.serialize();
    smsc_log_debug(logger, "Server sending %d bytes\n%s", http_packet.size(), http_packet.c_str());
    int n = sock->Write(http_packet.c_str(), http_packet.size());
    smsc_log_debug(logger, "Server sent %d bytes", n);
  }
*/

}//test
}//mms
}//transport
}//scag
