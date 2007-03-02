#ifndef __SCAG_MMS_PARSER_H__
#define __SCAG_MMS_PARSER_H__

#include <cstring>
#include <string.h>

#include "logger/Logger.h"
#include "sms/sms.h"

namespace scag{
namespace transport{
namespace mms{

static const char* POST = "POST";
static const char* HTTP = "HTTP";

static const char* PART_DELIMITER = "\r\n\r\n";
static const size_t PART_DELIMITER_SIZE = std::strlen(PART_DELIMITER);
static const char* CRLF = "\r\n";
static const size_t CRLF_SIZE = std::strlen(CRLF);
static const char* LF = "\n";
static const char* SPACE = " \r\n\t";

static const char* HOST = "Host";
static const char* CONTENT_TYPE = "Content-Type";
static const char* MULTIPART = "multipart/related";
static const size_t MULTIPART_SIZE = std::strlen(MULTIPART);
static const char* TEXT_XML = "text/xml";
static const size_t TEXT_XML_SIZE = std::strlen(TEXT_XML);
static const char* CHARSET = "charset";
static const size_t CHARSET_SIZE = std::strlen(CHARSET);


class HttpMsg {
  char* first_line;
  char* host_name;
  char* header;
  char* soap_envelope;
  char* soap_attach;
  char* envelope_charset;
  bool request;
  bool multipart;
public:
  HttpMsg():first_line(0), host_name(0), header(0), soap_envelope(0), soap_attach(0), envelope_charset(0),
            request(true), multipart(false) {};
  ~HttpMsg() {
    if (first_line) {
      delete[] first_line;
    }
    if (host_name) {
      delete[] host_name;
    }
    if (header) {
      delete[] header;
    }
    if (soap_envelope) {
      delete[] soap_envelope;
    }
    if (soap_attach) {
      delete[] soap_attach;
    }
    if (envelope_charset) {
      delete[] envelope_charset;
    }
  };
  static void fillField(char* &field, const char* src, int size = -1) {
    if (!src || size == 0) {
      return;
    }
    if (size < 0) {
      size = std::strlen(src);
    }
    if (field) {
      delete[] field;
    }
    field = new char[size + 1];
    memcpy(field, src, size);
    field[size] = 0;
  }
  void setFirstLine(const char* _first_line, int size = -1) {
    fillField(first_line, _first_line, size);
    if (std::strncmp(first_line, POST, std::strlen(POST)) == 0) {
      request = true;
    }
    if (std::strncmp(first_line, HTTP, std::strlen(HTTP)) == 0) {
      request = false;
    }
  }
  const char* getFirstLine() const {
    return first_line;
  }
  void setHostName(const char* _host_name, int size = -1) {
    fillField(host_name, _host_name, size);
  }
  const char* getHostName() const {
    return host_name;
  }
  void setHeader(const char* _header, int size = -1) {
    fillField(header, _header, size);
  }
  const char* getHeader() const {
    return header;
  }
  void setSoapEnvelope(const char* _soap_envelope, int size = -1) {
    fillField(soap_envelope, _soap_envelope, size);
  }
  const char* getSoapEnvelope() const {
    return soap_envelope;
  }
  void setSoapAttach(const char* _soap_attach, int size = -1) {
    fillField(soap_attach, _soap_attach, size);
  }
  const char* getSoapAttach() const {
    return soap_attach;
  }
  bool isRequest() const{
    return request;
  }
  void setRequest(bool _request) {
    request = _request;
  }
  bool isMultipart() const {
    return multipart;
  }
  void setMultipart(bool _multipart) {
    multipart = _multipart;
  }
  void setEnvelopeCharset(const char* charset, int size = -1) {
    fillField(envelope_charset, charset, size);
  }
  const char* getEnvelopeCharset() {
    return envelope_charset;
  }
};

struct HeaderField {
private:
  char* name;
  char* value;
  size_t value_size;
  size_t name_size;
public:
  HeaderField():name(0), value(0), name_size(0), value_size(0) {
  }
  ~HeaderField() {
    if (name) {
      delete[] name;
    }
    if (value) {
      delete[] value;
    }
  }
  void setName(const char* _name, int size = -1) {
    HttpMsg::fillField(name, _name, size);
    if (size < 0) {
      name_size = std::strlen(name);
    } else {
      name_size = size;
    }
  }
  const char* getName() const {
    return name;
  }
  void setValue(const char* _value, int size = -1) {
    HttpMsg::fillField(value, _value, size);
    if (size < 0) {
      value_size = std::strlen(value);
    } else {
      value_size = size;
    }
  }
  const char* getValue() const {
    return value;
  }
};

class MmsParser {
  static inline size_t getPartSize(const char* msg, size_t msg_size) {
    size_t part_size = 0;
    if (!msg || msg_size == 0) {
      return part_size;   
    }
    while (*msg) {
      if (part_size + PART_DELIMITER_SIZE < msg_size) {
        if (std::strncmp(msg, PART_DELIMITER, PART_DELIMITER_SIZE) == 0) {
	  return part_size;
	}
      }
      ++part_size;
      ++msg;
    }
    return part_size;
  }
  static bool charsetEnd(char ch) {
    if (!ch || ch == ' ' || ch == '\t' || ch == '\"' || ch == '\'' || ch == ';' || ch == ',') {
      return true;
    } else {
      return false;
    }
  }
  static inline bool getCharset(const char* content_type_string, HttpMsg& http_msg) {
    while (*content_type_string) {
      size_t space_count = std::strspn(content_type_string, " \t");
      content_type_string = content_type_string + space_count;
      size_t name_size = 0;
      while (*content_type_string && *content_type_string != '=') {
        ++content_type_string;
	++name_size;
	if (*content_type_string < 33 || *content_type_string > 126) {
	  name_size = 0;
	}
      }
      if (!*content_type_string) {
        return false;
      }
      ++content_type_string;
      --name_size;
      if (name_size == CHARSET_SIZE && std::strncmp(content_type_string - name_size - 1, CHARSET, CHARSET_SIZE) == 0) {
        //__trace2__("%s", content_type_string);
        size_t value_size = 0;
        space_count = std::strspn(content_type_string, " \t\"\'");
	content_type_string = content_type_string + space_count;
	while (!charsetEnd(*content_type_string)) {
	  ++value_size;
	  ++content_type_string;
	}
	http_msg.setEnvelopeCharset(content_type_string - value_size, value_size);
	return true;
      }
    }
    return true;
  }
  static inline int readLine(const char* msg, size_t msg_size, HeaderField& header_field) {
    if (!msg) {
      return -1;
    }
    if (std::strncmp(msg, CRLF, CRLF_SIZE) == 0) {
      return 0;
    }
    size_t line_size = 0;
    size_t space_count = std::strspn(msg, " \t");
    msg = msg + space_count;
    line_size += space_count;
    size_t name_size = 0;
    while(*msg && *msg != ':') {
      if (*msg < 33 || *msg > 126) {
        __trace__("MmsParser::readLine ERROR : Http header field name MUST be composed of charcters between 33 and 126 (RFC 2822 2.2)");
        return -1;
      }
      ++name_size;
      ++msg;
    }
    line_size += name_size + 1;
    if (line_size >= msg_size) {
      return -1;
    }
    header_field.setName(msg - name_size, name_size);
    ++msg; // before that *msg = ':'
    space_count = std::strspn(msg, " \t");
    line_size += space_count;
    msg += space_count;
    size_t value_size = 0;
    while(*msg) {
      if (std::strncmp(msg, CRLF, CRLF_SIZE) == 0) {
        header_field.setValue(msg - value_size, value_size);
        return line_size + value_size + CRLF_SIZE;
      }
      ++msg;
      ++value_size;
    }
    header_field.setValue(msg - value_size, value_size);
    return line_size + value_size;
  }
  static inline size_t parseHttpHeader(const char* msg, size_t msg_size, bool request, HttpMsg& http_msg) {
    size_t header_size = 0;
    int line_size = 1;
    size_t current_size = msg_size;
    while (*msg) {
      HeaderField header_field;
      line_size = readLine(msg, current_size, header_field);
      if (line_size == 0) {
        return header_size + CRLF_SIZE;
      }
      if (line_size == -1 ) {
        __trace__("MmsParser::parseHttpHeader : ERROR in Header Field format");
        return -1;
      }  
      current_size -= line_size;
      if (current_size < 0) {
        __trace__("MmsParser::parseHttpHeader : ERROR in HTTP Message format");
        return -1;
      }
      const char* field_name = header_field.getName();
      const char* field_value = header_field.getValue();
      __trace2__("Field Name : %s", field_name);
      __trace2__("Field Value : %s", field_value);
      if (request && std::strcmp(field_name, HOST) == 0) {
        http_msg.setHostName(field_value);
      }
      if (std::strcmp(field_name, CONTENT_TYPE) == 0) {
        if (request && field_value && (std::strncmp(field_value, MULTIPART, MULTIPART_SIZE) == 0)) {
          http_msg.setMultipart(true);
	}
	if (field_value && (std::strncmp(field_value, TEXT_XML, TEXT_XML_SIZE) == 0)) {
	  http_msg.setMultipart(false);
	  getCharset(field_value, http_msg);
	}
      }
      msg = msg + line_size;
      header_size += line_size;
    }
    return header_size;
  }
  static bool parseHttpResponse(const char* msg, size_t msg_size, HttpMsg& http_msg) {
    int part_size = parseHttpHeader(msg, msg_size, false, http_msg);
    if (part_size <= 0) {
      __trace__("MmsParser::parseHttpResponse : ERROR HTTP Header not Found");
      return false;
    }
    http_msg.setHeader(msg, part_size);
    msg = msg + part_size;
    size_t current_size = msg_size - part_size;
    part_size = getPartSize(msg, current_size);
    if (!part_size) {
      __trace__("MmsParser::parseHttpResponse : ERROR SOAP Envelope Not Found");
      return false;
    }
    http_msg.setSoapEnvelope(msg, part_size);
    return true;
  }
public:
  static bool parse(const char* msg, size_t msg_size, HttpMsg& http_msg) {
    if (!msg) {
      __trace__("MmsParser::parse : ERROR HTTP Message Not Found");
      return false;
    }
    size_t space_count = std::strspn(msg, SPACE);
    msg = msg + space_count;
    size_t line_size = std::strcspn(msg, LF) + 1;
    http_msg.setFirstLine(msg, line_size);
    msg = msg + line_size;
    size_t current_size = msg_size - space_count - line_size;
    if (!http_msg.isRequest()) {
      return parseHttpResponse(msg, current_size, http_msg);
    }
    //__trace__("Start Request Header Parse");
    int header_size = parseHttpHeader(msg, current_size, true, http_msg);
    //__trace__("End Request Header Parse");
    if (header_size <= 0) {
      __trace__("MmsParser::parse : ERROR HTTP Header Not Found");
      return false;
    }
    if (header_size == current_size) {
      __trace__("MmsParser::parse : ERROR SOAP Envelope Header Not Found");
      return false;
    }
    if (!http_msg.isMultipart()) {
      http_msg.setHeader(msg, header_size);
      msg = msg + header_size;
      current_size -= header_size;
      http_msg.setSoapEnvelope(msg, current_size);
      return true;
    }
//parse SOAP Envelope Header    
    line_size = std::strcspn(msg + header_size, LF) + 1;
    header_size += line_size;
    int part_header_size = parseHttpHeader(msg + header_size, current_size - header_size, false, http_msg);
    if (part_header_size <= 0) {
      __trace__("MmsParser::parse : ERROR SOAP Envelope Header Not Found");
      return false;
    }
    http_msg.setMultipart(true);
    header_size += part_header_size;
    http_msg.setHeader(msg, header_size);
    msg = msg + header_size;
    current_size -= header_size;
//parse SOAP Envelope    
    size_t envelope_size = getPartSize(msg, current_size);
    if (!envelope_size) {  
      __trace__("MmsParser::parse : ERROR SOAP Envelope Not Found");
      return false;
    } 
    http_msg.setSoapEnvelope(msg, envelope_size);
    msg = msg + envelope_size;
    current_size -= envelope_size;
//parse SOAP Attachment
    if (current_size) {
      http_msg.setSoapAttach(msg, current_size);
    }
    return true;
  }
};

}//mms
}//transport
}//scag

#endif

