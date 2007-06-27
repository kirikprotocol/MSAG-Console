#include "MmsParser.hpp"

namespace scag {
namespace transport {
namespace mms {

bool HttpPacket::parse(const char* buf, size_t buf_size) {
  if (!buf || buf_size == 0) {
    return size;
  }
  while (!complite && size < buf_size) {
    size_t _size = HttpParser::parse(buf, buf_size, this);
    size += _size;
    buf += _size;
  }
  if (complite && valid) {
    return true;
  } else {
    return false;
  }
};

void HttpPacket::setStartLine(const char* buf, size_t line_size) {
  if (!buf || line_size < HTTP_SIZE) {
    return;
  }
  if (std::strncmp(buf, HTTP, HTTP_SIZE) == 0) {
    string http_status(buf + HTTP_SIZE + HTTP_VERSION_SIZE, HTTP_STATUS_SIZE);
    int status_code = atoi(http_status.c_str()); 
    if ((status_code >= 100 && status_code < 200) || (status_code >= 300 && status_code < 600)){
      error_resp = true;
    }
    request = false;
  }
  start_line.erase();
  start_line.append(buf, line_size);
}

string HttpPacket::serialize() const {
  if (start_line.empty()) {
    return "";
  }
  string str(start_line);
  if (error_resp) {
    return str;
  }
  str += header.serialize();
  if (soap_envelope.empty()) {
    return "";
  }
  if (header.isMultipart()) {
    string boundary = header.getBoundary();
    if (boundary.empty()) {
      return "";
    }
    str += boundary;
    str += envelope_header.serialize();
    str += CRLF;
    str += soap_envelope;
    if (!soap_attachment.empty()) {
      str += boundary;
      str += soap_attachment;
    }
    str += header.getEndBoundary();
    return str;
  }
  str += CRLF;
  str += soap_envelope;
  return str;
}

void HttpPacket::test() {
  __trace__("TRACE HttpPacket");
  __trace2__("StartLine=\'%s\'", start_line.c_str());
  header.test();
  envelope_header.test();
  __trace2__("SoapEnvelope=\'%s\'", soap_envelope.c_str());
  __trace2__("SoapAttachment=\'%s\'", soap_attachment.c_str());
  __trace2__("Size=%d", size);
  __trace2__("ContentSize=%d", content_size);
  __trace2__("State=%d", state);
  __trace2__("NextState=%d", next_state);
  __trace2__("Complite=%d", complite);
  __trace2__("Valid=%d", valid);
  __trace2__("Request=%d", request);
  __trace2__("ErrorResp=%d", error_resp);
}

int HttpParser::findNextPacket(const char* buf, size_t buf_size, HttpPacket* packet) {
  size_t size = 0;
  while (*buf) {
    if ((std::strncmp(buf, POST, POST_SIZE) == 0) || (std::strncmp(buf, HTTP, HTTP_SIZE) == 0)) {
      packet->complite = true;
      return size;
    }
    ++buf;
    ++size;
  }
  return size;
}

int HttpParser::parseStartLine(const char* buf, size_t buf_size, HttpPacket* packet) {
  size_t size = 0;
  while (*buf && isspace(*buf)) {
    ++buf;
    ++size;
  }
  if (size == buf_size) {
    packet->valid = false;
    packet->complite = true;
    return size;
  }
  if ((std::strncmp(buf, POST, POST_SIZE) != 0) && (std::strncmp(buf, HTTP, HTTP_SIZE) != 0)) {
    packet->valid = false;
    packet->state = ERROR;
    return size;
  }
  size_t line_size = 0;
  while (*buf) {
    if (std::strncmp(buf, CRLF, CRLF_SIZE) == 0) {
      //packet->setStartLine(buf-line_size, line_size + CRLF_SIZE);
      line_size += CRLF_SIZE;
      buf += CRLF_SIZE;
      packet->state = HTTP_HEADER;
      break;
      //return size + line_size + CRLF_SIZE;
    }
    ++buf;
    ++line_size;
  }
  packet->setStartLine(buf-line_size, line_size);
  line_size += size;
  if (packet->isErrorResp()) {
    packet->complite = true;
  }
  return line_size;
}

int HttpHeader::parseHeaderLine(const char* buf, size_t buf_size) {
  if (std::strncmp(buf, CRLF, CRLF_SIZE) == 0) {
    return 0;
  }
  size_t line_size = 0;
  size_t space_count = std::strspn(buf, " \t");
  buf += space_count;
  line_size += space_count;
  size_t name_size = 0;
  while(*buf && *buf != ':') {
    if (*buf < 33 || *buf > 126) {
      //__trace__("MmsParser::readLine ERROR : Http header field name MUST be composed of charcters between 33 and 126 (RFC 2822 2.2)");
      while (*buf) {
        if (std::strncmp(buf, CRLF, CRLF_SIZE) == 0) {
          return name_size + line_size + CRLF_SIZE;
        }
        ++name_size;
        ++buf;
      }
      return  name_size + line_size;
    }
    ++name_size;
    ++buf;
  }
  line_size += name_size + 1;
  if (line_size >= buf_size) {
    return line_size;
  }
  const char* name_end = buf;
  ++buf;
  space_count = std::strspn(buf, " \t");
  line_size += space_count;
  buf += space_count;
  size_t value_size = 0;
  while (*buf){ 
    if (std::strncmp(buf, CRLF, CRLF_SIZE) == 0) {
      //header_field.setValue(msg - value_size, value_size);
      addField(name_end - name_size, name_size, buf - value_size, value_size);
      return line_size + value_size + CRLF_SIZE;
    }
    ++buf;
    ++value_size;
  }
  addField(name_end - name_size, name_size, buf - value_size, value_size);
  return line_size + value_size;
}

int HttpParser::parseHttpHeader(const char* buf, size_t buf_size, HttpPacket* packet) {
  size_t header_size = 0;
  while (*buf && isspace(*buf)) {
    ++header_size;
    ++buf;
  }
  while (*buf) {
    size_t line_size = packet->header.parseHeaderLine(buf, buf_size - header_size);
    if (line_size == 0) {
      if (!packet->header.parseContentType()) {
        packet->state = ERROR;
        packet->valid = false;
        return header_size + CRLF_SIZE;
      }
      if (packet->header.isMultipart()) {
        packet->state = FIND_NEXT_PART;
        packet->next_state = ENVELOPE_HEADER;
        return header_size;
      } 
      buf += CRLF_SIZE;
      header_size += CRLF_SIZE;
      size_t content_length = packet->header.getContentLength();
      size_t envelope_size = buf_size - packet->getSize() - header_size;
      packet->valid = true;
      packet->complite = true;
      if (content_length) {
        if (content_length > envelope_size) {
          packet->valid = false;
          packet->complite = false;
        } else {
          envelope_size = content_length;
        }
      }
      packet->state = SOAP_ENVELOPE;
      packet->setSoapEnvelope(buf, envelope_size);
      return header_size + envelope_size;
    }
    header_size += line_size;
    buf += line_size;
  }
  return header_size;
}

int HttpParser::parseEnvelopeHeader(const char* buf, size_t buf_size, HttpPacket* packet) {
  size_t header_size = 0;
  while (*buf && isspace(*buf)) {
    ++header_size;
    ++buf;
  }
  while (*buf) {
    size_t line_size = packet->envelope_header.parseHeaderLine(buf, buf_size - header_size);
    if (line_size == 0) {
      if (packet->envelope_header.parseContentType()) {
        packet->state = SOAP_ENVELOPE;
      } else {
        packet->state = ERROR;
        packet->valid = false;
      }
      packet->content_size += header_size + CRLF_SIZE;
      return header_size + CRLF_SIZE;
    }
    header_size += line_size;
    buf += line_size;
  }
  packet->content_size += header_size;
  return header_size;
}

int HttpParser::parseSoapEnvelope(const char* buf, size_t buf_size, HttpPacket* packet) {
  size_t envelope_size = 0;
  const char* boundary = packet->header.getBoundary().c_str();
  size_t boundary_size = std::strlen(boundary);
  while (*buf) {
    if (std::strncmp(buf, boundary, boundary_size) == 0)  {
      //__trace2__("BUF=\'%s\'", buf);
      if (std::strncmp(buf + boundary_size, BOUNDARY_START, BOUNDARY_START_SIZE) == 0) {
        packet->complite = true;
        packet->valid = false;
      }
      packet->state = SOAP_ATTACHMENT;
      packet->setSoapEnvelope(buf - envelope_size, envelope_size);
      packet->content_size += envelope_size + boundary_size;
      return envelope_size + boundary_size;
    }
    ++buf;
    ++envelope_size;
  }
  packet->content_size += envelope_size;
  packet->setSoapEnvelope(buf - envelope_size, envelope_size);
  int content_length = packet->header.getContentLength();
  if (content_length > 0 && content_length <= packet->content_size) {
    packet->valid = false;
  }
  return envelope_size;
}

int HttpParser::parseSoapAttachment(const char* buf, size_t buf_size, HttpPacket* packet) {
  int content_length = packet->header.getContentLength();
  if ((content_length > 0) && ((content_length -= packet->getContentSize()) <= 0)) {
    //size_t attachment_size = buf_size - packet->getSize();
    packet->valid = false;
    return buf_size - packet->getSize();
  }
    //if (content_length <= attachment_size) {
      //packet->valid = true;
      //packet->complite = true;
      //attachment_size = content_length;
    //}
    //packet->setSoapAttachment(buf, attachment_size);
    //packet->content_size += attachment_size;
    //return attachment_size;
  //}
  size_t attachment_size = 0;
  string end_boundary = packet->header.getEndBoundary();
  size_t end_boundary_size = end_boundary.size();
  while (*buf) {
    if (std::strncmp(buf, end_boundary.c_str(), end_boundary_size) == 0) {
      packet->complite = true;
      //packet->valid = true;
      packet->setSoapAttachment(buf - attachment_size, attachment_size);
      packet->content_size += attachment_size;
      if (content_length > 0 && attachment_size > content_length) {
        packet->valid = false;
      }
      return attachment_size;
    }
    ++buf;
    ++attachment_size;
  }
  if (content_length > 0 && attachment_size > content_length) {
    packet->valid = false;
  }
  packet->setSoapAttachment(buf - attachment_size, attachment_size);
  packet->content_size += attachment_size;
  return attachment_size;
}

int HttpParser::findNextPart(const char* buf, size_t buf_size, HttpPacket* packet) {
  size_t size = 0;
  string boundary = packet->header.getBoundary();
  size_t boundary_size = boundary.size();
  while (*buf) {
    if (std::strncmp(buf, boundary.c_str(), boundary_size) == 0) {
      packet->state = packet->next_state;
      return size + boundary_size;
    }
    ++buf;
    ++size;
  }
  return size;
}

void HttpHeader::setContentType(const char* type) {
  if (std::strncmp(type, MULTIPART, MULTIPART_SIZE) == 0) {
    multipart = true;
  }
  content_type = type;
}

bool HttpHeader::parseContentType() {
  const string* ptr = fields.GetPtr(CONTENT_TYPE);
  if (!ptr) {
    return false;
  }
  string content_type_string(*ptr);
  fields.Delete(CONTENT_TYPE);
  size_t pos = 0;
  size_t next_pos = 0;
  while (next_pos != string::npos) {
    next_pos = content_type_string.find_first_of(';', pos);
    string part(content_type_string, pos, next_pos - pos);
    size_t sep_pos = part.find_first_of('=');
    string name(trimString(string(part, 0, sep_pos), SPACES));
    if (name.empty()) {
      break;
    }
    for (string::iterator i = name.begin(); i != name.end(); ++i) {
      *i = tolower(*i);
    }
    if (0 == pos) {
      setContentType(name.c_str());
      pos = next_pos + 1;
      continue;
    }
    string value = sep_pos != string::npos ? trimString(string(part, sep_pos + 1, string::npos), SPACES_QOUTES) : "";
    content_type_params.Insert(name.c_str(), value);
    pos = next_pos + 1;
  }
  if (multipart) {
    ptr = content_type_params.GetPtr(BOUNDARY);
    if (!ptr || (*ptr).empty()) {
      return false;
    }
    setBoundary(*ptr);
    content_type_params.Delete(BOUNDARY);
  } 
  ptr = content_type_params.GetPtr(CHARSET);
  if (ptr) {
    charset = *ptr;
    content_type_params.Delete(CHARSET);
  }
  return true;
}

string HttpHeader::serializeContentTypeParam(const char* param_name, string param_value) const {
  string str = "; ";
  str += param_name;
  str += "=\"" + param_value + "\"";
  return str;
}

string HttpHeader::serializeContentType() const {
  string str;
  str += content_type;
  if (!boundary.empty()) {
    string _boundary = trimString(boundary, "\r\n--");
    str += serializeContentTypeParam(BOUNDARY, trimString(boundary, "\r\n--"));
  }
  if (!charset.empty()) {
    str += serializeContentTypeParam(CHARSET, charset);
  }
  Hash<string>::Iterator i = content_type_params.getIterator();
  i.First();
  char* name = 0;
  string value;
  while(i.Next(name, value)) {
    str += serializeContentTypeParam(name, value);
  }
  return str;
}

string HttpHeader::serializeHeaderField(const char* field_name, string field_value) const {
  string str(field_name);
  str += ": " + field_value + CRLF;
  return str;
}

string HttpHeader::serialize() const {
  string str = serializeHeaderField(CONTENT_TYPE, serializeContentType());
  Hash<string>::Iterator i = fields.getIterator();
  i.First();
  char* name = 0;
  string value;
  while(i.Next(name, value)) {
    str += serializeHeaderField(name, value);
  }
  return str;
}

void HttpHeader::test() {
  __trace__("TRACE HttpHeader");
  __trace2__("charset=\'%s\'", charset.c_str());
  __trace2__("boundary=\'%s\'", boundary.c_str());
  __trace2__("content_type=\'%s\'", content_type.c_str());
  fields.First();
  char* name = 0;
  string value;
  while(fields.Next(name, value)) {
    __trace2__("Field Name = \'%s\' Field Value = \'%s\'", name, value.c_str());
  }
  if (multipart) {
    __trace__("Multipart=true");
  } else {
    __trace__("Multipart=false");
  }
  __trace2__("content_length=%d", content_length);
  string ser_header = serialize();
  __trace2__("SerializedHeader:\n\'%s\'", ser_header.c_str());
}

}//mms
}//transport
}//scag