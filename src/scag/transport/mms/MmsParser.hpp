#ifndef __SCAG_MMS_PARSER_HPP__
#define __SCAG_MMS_PARSER_HPP__

#include <cstring>
#include <vector>
#include <string.h>
#include <stdlib.h>

#include "logger/Logger.h"
#include "sms/sms.h"
#include "core/buffers/Hash.hpp"

#include "util.h"

namespace scag{
namespace transport{
namespace mms{

using std::string;
using std::vector;

using smsc::logger::Logger;
using smsc::core::buffers::Hash;

//static const char* POST = "POST";
//static const char* HTTP = "HTTP";

static const int BAD_REQUEST = 400;
static const int INTERNAL_SERVER_ERROR = 500;
static const int SERVICE_UNAVAILABLE = 503;

static const char* OK_RESPONSE = "HTTP/1.1 200 Ok\r\n";
static const char* POST = "POST";
static const char* HTTP = "HTTP";
static const size_t POST_SIZE = std::strlen(POST);
static const size_t HTTP_SIZE = std::strlen(HTTP);

static const char* PART_DELIMITER = "\r\n\r\n";
static const size_t PART_DELIMITER_SIZE = std::strlen(PART_DELIMITER);
static const char* CRLF = "\r\n";
static const size_t CRLF_SIZE = std::strlen(CRLF);
static const char* LF = "\n";
static const char* CR = "\r";
//static const char* SPACE = " \r\n\t";
static const char* COLON = ": ";
  
static const char* HOST = "Host";
static const size_t HOST_SIZE = std::strlen(HOST);
static const char* CONTENT_TYPE = "Content-Type";
static const char* TEXT_XML = "text/xml";
static const char* START = "start";
static const char* TYPE = "type";
static const size_t TEXT_XML_SIZE = std::strlen(TEXT_XML);
static const char* CHARSET = "charset";
static const size_t CHARSET_SIZE = std::strlen(CHARSET);
static const char* BOUNDARY = "boundary";
static const size_t BOUNDARY_SIZE = std::strlen(BOUNDARY);

static const char* HTTP_VERSION = "/1.1 ";
static size_t HTTP_VERSION_SIZE = std::strlen(HTTP_VERSION);
static size_t HTTP_STATUS_SIZE = 3;

static const char* LFCRLF = "\n\r\n";
static size_t LFCRLF_SIZE = std::strlen(HTTP_VERSION);

static const char* ENVELOPE_END = "</env:Envelope>";
static size_t ENVELOPE_END_SIZE = std::strlen(ENVELOPE_END);

static const char* SPACES_QOUTES = " \n\t\'\"";
static const char* SPACES = " \n\t";
static const char* BOUNDARY_START = "--";
static const size_t BOUNDARY_START_SIZE = std::strlen(BOUNDARY_START);

static const char* CONTENT_LENGTH = "Content-Length";
static const char* SAMPLE_CONTENT_LENGTH = "nnnn";
static const size_t CONTENT_LENGTH_SIZE = 14;

static const char* SOAP_ACTION = "SOAPAction";


struct HttpHeader {
public:
  HttpHeader():multipart(false), content_length(0) {
    logger = Logger::getInstance("mms.parser");
  }
  virtual ~HttpHeader() {}
  void serialize(string& serialized_header) const;

  bool isMultipart() const;
  void addField(const char* name_buf, size_t name_size, 
                const char* value_buf, size_t value_size);
  void addField(const char* name, string value);
  size_t getContentLength();
  bool parseContentType();
  void setBoundary(const string& _boundary);
  const string& getBoundary() const;
  void setCharset(const char* _charset);
  void addContentTypeParam(const char* param_name, string param_value);
  string getHost() const;
  void getEndBoundary(string& end_boundary) const;
  void setContentType(const char* type);
  int parseHeaderLine(const char* buf, size_t buf_size);
  void test();
  void clear();
private:
  void serializeContentType(string& serialized) const;
  void serializeContentTypeParam(const char* param_name, const string& param_value,
                                 string& serialized) const;
  void serializeHeaderField(const char* field_name, const string& field_value,
                            string& serialized) const; 
private:
  string charset;
  string content_type;
  string boundary;
  size_t content_length;
  Hash<string> fields;
  Hash<string> content_type_params;
  bool multipart;
  Logger* logger;
};

enum HttpPacketState {
  START_LINE,      //0
  HTTP_HEADER,     //1
  ENVELOPE_HEADER, //2 
  SOAP_ENVELOPE,   //3
  SOAP_ATTACHMENT, //4 
  ERROR,           //5
  FIND_NEXT_PART   //6
};

class HttpParser;

class HttpPacket {
public:
  HttpPacket():size(0), complite(false), valid(true), request(true), error_resp(false), packet_size(0),
               state(START_LINE), next_state(HTTP_HEADER), content_size(0), modified(true) {
    logger = Logger::getInstance("mms.parser");
  }
  ~HttpPacket() {
  }
  bool parse(const char* buf, size_t buf_size);
  void setStartLine(const char* buf, size_t line_size);
  size_t getSize() const;
  void setContentLength(size_t length);
  size_t getContentSize() const;
  uint8_t getState() const;
  bool isValid() const;
  bool isComplite() const;
  bool isRequest() const;
  bool isErrorResp() const;
  void setSoapEnvelope(const char* buf, size_t envelope_size);
  void setSoapEnvelope(const string& envelope);
  const char* getSoapEnvelope() const;
  size_t getSoapEnvelopeSize() const;
  void setSoapAttachment(const char* buf, size_t attachment_size);
  void setHttpHeader(const HttpHeader& _header);
  void setEnvelopeHeader(const HttpHeader& _header);
  void setHost(const string& host);
  string getHost() const;
  const char* getPacket();
  size_t getPacketSize() const;
  bool serialize(string& serialized_packet) const;
  void createFakeResp(int status);
  void fillResponse();

  void test();
  void clear();
private:
  HttpHeader header;
  HttpHeader envelope_header;
  string soap_envelope;
  string soap_attachment;
  string start_line;
  string packet;
  size_t packet_size;
  size_t size;
  size_t content_size;
  uint8_t state;
  uint8_t next_state;
  bool complite;
  bool valid;
  bool request;
  bool error_resp;
  bool modified;
  Logger* logger;
private:
  friend class HttpParser;

};

class HttpParser {
public:
  static int parse(const char* buf, size_t buf_size, HttpPacket* packet) {
    switch (packet->getState()) {
      case START_LINE      : return parseStartLine(buf, buf_size, packet);
      case HTTP_HEADER     : return parseHttpHeader(buf, buf_size, packet);
      case ENVELOPE_HEADER : return parseEnvelopeHeader(buf, buf_size, packet);
      case SOAP_ENVELOPE   : return parseSoapEnvelope(buf, buf_size, packet);
      case SOAP_ATTACHMENT : return parseSoapAttachment(buf, buf_size, packet);
      case ERROR           : return buf_size - packet->getSize();//findNextPacket(buf, buf_size, packet);
      case FIND_NEXT_PART  : return findNextPart(buf, buf_size, packet);
    }
    return 0;
  }
private:
  static inline void fillField(char* &field, const char* src, int size = -1) {
    if (!src || size == 0) {
      return;
    }
    if (size < 0) {
      size = strlen(src);
    }
    if (field) {
      delete[] field;
    }
    field = new char[size + 1];
    memcpy(field, src, size);
    field[size] = 0;
  }
  static int parseStartLine(const char* buf, size_t buf_size, HttpPacket* packet);
  static int parseHttpHeader(const char* buf, size_t buf_size, HttpPacket* packet);
  static int parseEnvelopeHeader(const char* buf, size_t buf_size, HttpPacket* packet);
  static int parseSoapEnvelope(const char* buf, size_t buf_size, HttpPacket* packet);
  static int parseSoapAttachment(const char* buf, size_t buf_size, HttpPacket* packet);
  static int findNextPacket(const char* buf, size_t buf_size, HttpPacket* packet);
  static int findNextPart(const char* buf, size_t buf_size, HttpPacket* packet);
};

static string trimString(const string& s, const char* what_trim) {
  size_t start = s.find_first_not_of(what_trim);
  if (string::npos == start) {
    return string("");
  }
  size_t end = s.find_last_not_of(what_trim);
  return string(s, start, end - start + 1);
}

}//mms
}//transport
}// scag

#endif
 

