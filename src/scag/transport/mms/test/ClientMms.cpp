#include "ClientMms.h"

namespace scag { namespace transport { namespace mms { namespace test {

const char* START = "start";
const char* TYPE = "type";
const size_t CONTENT_LENGTH_SIZE = 14;
const char* SAMPLE_POST = "POST /mms-rs/mm7 HTTP/1.1\r\n";
const size_t SAMPLE_POST_SIZE = strlen(SAMPLE_POST);
const size_t OK_RESPONSE_SIZE = strlen(OK_RESPONSE);
const char* SAMPLE_HOST = "msag";
const char* CONTENT_ID = "Content-ID";
const char* TID_TEMPLATE = "@tid@";
const char* EPID_TEMPLATE = "@endpoint_id@";
const size_t TID_TEMPLATE_SIZE = strlen(TID_TEMPLATE);
const size_t EPID_TEMPLATE_SIZE = strlen(EPID_TEMPLATE);
const char* MULTIPART = "multipart";
const size_t MULTIPART_SIZE = strlen(MULTIPART);

const int SERVER_TIME_OUT = 10;
const size_t CLIENT_TIME_OUT = 100;
const char* SAMPLE_BOUNDARY = "NextPart_000_0128_01C19839.12345";
const char* SAMPLE_START = "</tnn-123456/mm7-request>";

//public
bool ClientMms::start() {
  smsc_log_debug(logger, "Start MMS Client");
  smsc_log_debug(logger, "MMS Relay Server \'%s:%d\'", host.c_str(), port);
  if (socket.Init(host.c_str(),port, CLIENT_TIME_OUT) < 0) {
    smsc_log_error(logger, "Can't Init Socket on host = \'%s\' port = %d", host.c_str(), port);
    return false;
  }
  return true;
}
  
bool ClientMms::sendCommand(const string& cmd_template, const string& soap_attchment) {
  MmsCommand cmd;
  if (!cmd.createMM7Command(cmd_template.c_str(), cmd_template.size())) {
    return false;
  }
  cmd.setTransactionId(getTransactionId(prefix));
  if (is_vasp) {
    cmd.setInfoElement(xml::VASP_ID, endpoint_id);
  } else {
    cmd.setInfoElement(xml::MMS_RS_ID, endpoint_id);
  }
  return sendVASPRequest(cmd, soap_attchment);
}

void ClientMms::sendRequestWithPrintf(const char* packet) {
  sendRequestPacket(packet) ? printf("\nSuccess Send\n") : printf("\nError Send\n");
}

bool ClientMms::flood(string& cmd_template, const string& soap_attachment, const Hash<string>& fields, 
             const vector<string>& values, const char* template_string) {
  int n = values.size();
  if (n == 0) {
    return false;
  }
  HttpPacket packet;
  if (soap_attachment.empty()) {
    createRequestPacket(packet);
  } else {
    createMultipartRequestPacket(packet);
    packet.setSoapAttachment(soap_attachment.c_str(), soap_attachment.size());
  }
  size_t epid_pos = cmd_template.find(EPID_TEMPLATE);
  if (epid_pos != string::npos) {
    cmd_template.replace(epid_pos, EPID_TEMPLATE_SIZE, endpoint_id);
  }
  size_t templ_size = strlen(template_string);
  size_t tid_pos = cmd_template.find(TID_TEMPLATE);
  size_t pos = cmd_template.find(template_string);
  for (vector<string>::const_iterator i = values.begin(); i != values.end(); ++i) {
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
    sendRequestWithPrintf(packet.getPacket());
  }
  return true;
}

bool ClientMms::sendCommand(const string& cmd_template, const string& soap_attchment,
                           const Hash<string>& fields) {
  MmsCommand cmd;
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

bool ClientMms::sendRequestPacket(const char* packet) {
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
  int part_size = 512;
  int n = socket.Write(packet, part_size);
  //sleep(100);
  n += socket.Write(packet + n, packet_size - n);

  //int n = socket.Write("", 0);
  smsc_log_debug(logger, "Request Packet Size = %d", packet_size);
  smsc_log_debug(logger, "Client Sent %d Bytes", n);
  smsc_log_debug(logger, "Request Packet:\n\n\'%s\'\n", packet);
  smsc_log_debug(logger, "Start Receive Response Packet");
  std::string in_packet;
  recvPacket(&socket, SERVER_TIME_OUT * 10, in_packet);
  smsc_log_debug(logger, "Response Packet size=%d:\n\n\'%s\'\n"
                 ,in_packet.size(), in_packet.c_str());
  /********************/
  socket.Close();
  /********************/
  HttpPacket http_packet;
  if (!http_packet.parse(in_packet.c_str(), in_packet.size())) {
    smsc_log_error(logger, "Error While Parsing Response Packet");
    http_packet.test();
    return false;
  }
  if (http_packet.isErrorResp()) {
    smsc_log_debug(logger, "Client Recive Error Resp");
    return true;
  }
  MmsCommand in_cmd;
  //string soap_env = http_packet.getSoapEnvelope();
  smsc_log_debug(logger, "Create Response MM7 Command");
  if (in_cmd.createMM7Command(http_packet.getSoapEnvelope(),
                               http_packet.getSoapEnvelopeSize())) {
    string serialized_resp;
    in_cmd.serialize(serialized_resp);
    smsc_log_debug(logger, "Serialized Response Command:\n\n\'%s\'\n",
                    serialized_resp.c_str());
  } else {
    smsc_log_error(logger, "Can't Create Response MM7 Command");
  }
  return true;
}

void ClientMms::closeSocket() {
  socket.Close();
}

//protected

void ClientMms::recvPacket(Socket* _socket, int time_out, string& packet) {
  const size_t buf_size = 1024;
  char buf[buf_size];
  int n = 0;
  _socket->setTimeOut(time_out);
  while (_socket->canRead() && ((n = _socket->Read(buf, buf_size)) > 0)) {
    packet.append(buf, n);
    if (n < buf_size) {
      return;
    }
  }
}

void ClientMms::createRequestPacket(HttpPacket& packet, size_t content_length) {
  HttpHeader header;
  header.setContentType(TEXT_XML);
  header.setCharset(xml::UTF_8);
  //header.addField(HOST, SAMPLE_HOST);
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
  packet.setHttpHeader(header);
  packet.setHost(SAMPLE_HOST);
  packet.setStartLine(SAMPLE_POST, SAMPLE_POST_SIZE);
}

void ClientMms::createResponsePacket(HttpPacket& packet, size_t content_length) {
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
  packet.setHttpHeader(header);
  packet.setStartLine(OK_RESPONSE, OK_RESPONSE_SIZE);
}

void ClientMms::createMultipartRequestPacket(HttpPacket& packet, size_t content_length, 
                                   const char* boundary, const char* start) {
  HttpHeader header;
  header.setBoundary(boundary);
  header.setContentType(MULTIPART);
  header.addContentTypeParam(START, start);
  header.addContentTypeParam(TYPE, TEXT_XML);
  //header.addField(HOST, SAMPLE_HOST);
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

  packet.setEnvelopeHeader(envelope_header);
  packet.setHost(SAMPLE_HOST);
  packet.setHttpHeader(header);
  packet.setStartLine(SAMPLE_POST, SAMPLE_POST_SIZE);
}

void ClientMms::setPrefix() {
  prefix = is_vasp ? "VASP_" : "RS_";
}

//private

bool ClientMms::sendVASPRequest(const MmsCommand& cmd, const string& soap_attchment) {
  string soap_envelope;
  if (!cmd.serialize(soap_envelope)) {
    return false;
  }
  if (soap_attchment.empty()) {
    HttpPacket packet;
    createRequestPacket(packet, soap_envelope.size());
    packet.setSoapEnvelope(soap_envelope.c_str(), soap_envelope.size());
    return sendRequestPacket(packet.getPacket());
  }
  HttpPacket packet;
  createMultipartRequestPacket(packet);
  packet.setSoapEnvelope(soap_envelope.c_str(), soap_envelope.size());
  packet.setSoapAttachment(soap_attchment.c_str(), soap_attchment.size());
  return sendRequestPacket(packet.getPacket());  
}

}//test
}//mms
}//transport
}//scag
