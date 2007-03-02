#include <iostream>
#include <fstream>

#include "MmsParser.h"
#include "logger/Logger.h"
#include "sms/sms.h"

using scag::transport::mms::MmsParser; 
using scag::transport::mms::HttpMsg;

int main(int argc, const char* argv[]) {
  smsc::logger::Logger::Init();
  if (argc < 2) {
    __trace__("Usage: parser_test http_msg");
    return 0;
  }
  std::ifstream f(argv[1]);
  if (!f) {
    __trace2__("Can't open file \"%s\"", argv[1]);
    return 0;
  }
  std::string msg;
  getline(f, msg, static_cast<char>(EOF));
  HttpMsg http_msg;
  if (!MmsParser::parse(msg.c_str(), msg.size(), http_msg)) {
    __trace__("PARSE ERROR!");
    return 0;
  }
  __trace2__("Http request/response line : %s", http_msg.getFirstLine());
  if (http_msg.isRequest()) {
    __trace2__("Host name : %s", http_msg.getHostName());
    if (http_msg.isMultipart()) {
      __trace__("Http message is multipart ");
    } 
  }
  __trace2__("SOAP Header : %s", http_msg.getHeader());
  __trace2__("SOAP Envelope : %s", http_msg.getSoapEnvelope());
  if (http_msg.getEnvelopeCharset())
    __trace2__("Envelope Charset : %s", http_msg.getEnvelopeCharset());
  if (http_msg.getSoapAttach()) 
    __trace2__("SOAP Attach : %s", http_msg.getSoapAttach());
  
  return 1;   
}
