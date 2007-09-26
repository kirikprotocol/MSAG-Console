#include "MmsParser.hpp"

namespace scag {
namespace transport {
namespace mms {

bool HttpPacket::parse(const char* buf, size_t buf_size) {
  //clear();
  if (!buf || !buf_size) {
    //__trace__("BUF = NULL");
    return false;
  }
  while (*buf && !complite && state != ERROR) {
    size_t _size = HttpParser::parse(buf, buf_size, this);
    size += _size;
    buf += _size;
   // __trace2__("BUF SIZE=%d BUF=\'%s\'", size, buf);
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
  modified = true;
  if (strncmp(buf, HTTP, HTTP_SIZE) == 0) {
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

bool HttpPacket::serialize(string& serialized_packet) const {
  if (start_line.empty()) {
    return false;
  }
  serialized_packet.erase();
  serialized_packet.append(start_line);
  if (error_resp) {
    return true;
  }
  header.serialize(serialized_packet);
  if (soap_envelope.empty()) {
    return false;
  }
  if (header.isMultipart()) {
    string boundary = header.getBoundary();
    if (boundary.empty()) {
      return false;
    }
    serialized_packet.append(boundary);
    envelope_header.serialize(serialized_packet);
    serialized_packet.append(CRLF);
    serialized_packet.append(soap_envelope);
    if (!soap_attachment.empty()) {
      serialized_packet.append(boundary);
      serialized_packet.append(soap_attachment);
    }
    serialized_packet.append(boundary, 0, boundary.size() - CRLF_SIZE);
    serialized_packet.append(BOUNDARY_START);
    return true;
  }
  serialized_packet.append(CRLF);
  serialized_packet.append(soap_envelope);
  return true;
}

size_t HttpPacket::getSize() const {
  return size;
}

size_t HttpPacket::getContentSize() const {
  return content_size;  
}

uint8_t HttpPacket::getState() const {
  return state;
}

bool HttpPacket::isValid() const {
  return valid;
}

bool HttpPacket::isComplite() const {
  return complite;
}

bool HttpPacket::isRequest() const {
  return request;
}

bool HttpPacket::isErrorResp() const {
  return error_resp;
}

void HttpPacket::setSoapEnvelope(const char* buf, size_t envelope_size) {
  soap_envelope.append(buf, envelope_size);
  setContentLength(envelope_size);
  modified = true;
}

void HttpPacket::setSoapEnvelope(const string& envelope) {
  soap_envelope = envelope;
  setContentLength(soap_envelope.size());
  modified = true;
}

const char* HttpPacket::getSoapEnvelope() const {
  return soap_envelope.c_str();
}

size_t HttpPacket::getSoapEnvelopeSize() const {
  return soap_envelope.size();
}

void HttpPacket::setSoapAttachment(const char* buf, size_t attachment_size) {
  soap_attachment.append(buf, attachment_size);
  setContentLength(0);
  modified = true;
}

void HttpPacket::setHttpHeader(const HttpHeader& _header) {
  header = _header;
  modified = true;
}

void HttpPacket::setEnvelopeHeader(const HttpHeader& _header) {
  envelope_header = _header;
  modified = true;
}

void HttpPacket::setHost(const string& host) {
  header.addField(HOST, host);
  modified = true;
}

string HttpPacket::getHost() const {
  return header.getHost();
}

const char* HttpPacket::getPacket() {
  if (modified) {
    modified = false;
    serialize(packet);
  }
  return packet.c_str();
}

size_t HttpPacket::getPacketSize() const {
  return packet.size();
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
    if ((strncmp(buf, POST, POST_SIZE) == 0) || (strncmp(buf, HTTP, HTTP_SIZE) == 0)) {
      packet->complite = true;
      return size;
    }
    ++buf;
    ++size;
  }
  return size;
}

int HttpParser::parseStartLine(const char* buf, size_t buf_size, HttpPacket* packet) {
  __trace__("parseStartLine");
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
  if ((strncmp(buf, POST, POST_SIZE) != 0) && (strncmp(buf, HTTP, HTTP_SIZE) != 0)) {
    packet->valid = false;
    packet->state = ERROR;
    return size;
  }  
  size_t line_size = 0;
  while (*buf) {
    if (strncmp(buf, CRLF, CRLF_SIZE) == 0) {
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

void HttpPacket::createFakeResp(int status) {
  modified = true;
  switch (status) {
  case BAD_REQUEST           : start_line = "HTTP/1.1 400 Bad Request\r\n"; break;
  case INTERNAL_SERVER_ERROR : start_line = "HTTP/1.1 500 Internal Server Error\r\n"; break;
  case SERVICE_UNAVAILABLE   : start_line = "HTTP/1.1 503 Service Unavailable\r\n"; break;
  }
  error_resp = true;
}
void HttpPacket::setContentLength(size_t length) {
  if (header.isMultipart() || 0 == length) {
    header.addField(CONTENT_LENGTH, SAMPLE_CONTENT_LENGTH);
    return;
  }
  const size_t buf_size = 14;
  char buf[buf_size];
  int n = snprintf(buf, buf_size, "%d", length);
  if (n > 0) {
    string value(buf, n);
    header.addField(CONTENT_LENGTH, value);
  }
}
void HttpPacket::fillResponse() {
  header.setContentType(TEXT_XML);
  header.setCharset(xml::UTF_8);
  header.addField(SOAP_ACTION, "\"\"");
  //setContentLength(soap_envelope.size());
  start_line = OK_RESPONSE;
  modified = true;
}

void HttpPacket::clear() {
  header.clear();
  envelope_header.clear();
  soap_envelope.erase();
  soap_attachment.erase();
  start_line.erase();
  packet.erase();
  packet_size = 0;
  size = 0;
  content_size = 0;
  state = START_LINE;
  next_state = HTTP_HEADER;
  complite = false;;
  valid = true;
  request = true;
  error_resp = false;
  modified = true;
}

int HttpParser::parseHttpHeader(const char* buf, size_t buf_size, HttpPacket* packet) {
  __trace__("parseHttpHeader");
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
          //packet->valid = false;
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
  __trace__("parseEnvelopeHeader");
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
  __trace__("parseSoapEnvelope");
  if (!packet->header.isMultipart()) {
    __trace__("NOT Multipart");
    packet->setSoapEnvelope(buf, buf_size);
    int content_length = packet->header.getContentLength();
    if (content_length > 0) {
      if (packet->soap_envelope.size() > content_length) {
        packet->valid = false;
      }
      if (packet->soap_envelope.size() == content_length) {
        packet->complite = true;
      }
    }
    return buf_size;
  }
  __trace__("Multipart");
  size_t envelope_size = 0;
  const char* boundary = packet->header.getBoundary().c_str();
  size_t boundary_size = strlen(boundary);
  while (*buf) {
    if (strncmp(buf, boundary, boundary_size) == 0)  {
      if (strncmp(buf + boundary_size, BOUNDARY_START, BOUNDARY_START_SIZE) == 0) {
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
  __trace__("parseSoapAttachment");
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
  string end_boundary;
  packet->header.getEndBoundary(end_boundary);
  size_t end_boundary_size = end_boundary.size();
  while (*buf) {
    if (strncmp(buf, end_boundary.c_str(), end_boundary_size) == 0) {
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
    if (strncmp(buf, boundary.c_str(), boundary_size) == 0) {
      packet->state = packet->next_state;
      return size + boundary_size;
    }
    ++buf;
    ++size;
  }
  return size;
}

int HttpHeader::parseHeaderLine(const char* buf, size_t buf_size) {
  if (strncmp(buf, CRLF, CRLF_SIZE) == 0) {
    return 0;
  }
  size_t line_size = 0;
  size_t space_count = strspn(buf, " \t");
  buf += space_count;
  line_size += space_count;
  size_t name_size = 0;
  while(*buf && *buf != ':') {
    if (*buf < 33 || *buf > 126) {
      //__trace__("MmsParser::readLine ERROR : Http header field name MUST be composed of charcters between 33 and 126 (RFC 2822 2.2)");
      while (*buf) {
        if (strncmp(buf, CRLF, CRLF_SIZE) == 0) {
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
  space_count = strspn(buf, " \t");
  line_size += space_count;
  buf += space_count;
  size_t value_size = 0;
  while (*buf){ 
    if (strncmp(buf, CRLF, CRLF_SIZE) == 0) {
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

void HttpHeader::setContentType(const char* type) {
  if (strncmp(type, MULTIPART, MULTIPART_SIZE) == 0) {
    multipart = true;
  }
  multipart = false;
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

void HttpHeader::serializeContentTypeParam(const char* param_name, const string& param_value,
                                           string& serialized) const {
  serialized.append("; ");
  serialized.append(param_name);
  serialized.append("=\"");
  serialized.append(param_value);
  serialized.append("\"");
}

void HttpHeader::serializeContentType(string& serialized) const {
  serialized.append(content_type);
  if (!boundary.empty()) {
    string _boundary = trimString(boundary, "\r\n--");
    serializeContentTypeParam(BOUNDARY, _boundary, serialized);
  }
  if (!charset.empty()) {
    serializeContentTypeParam(CHARSET, charset, serialized);
  }
  Hash<string>::Iterator i = content_type_params.getIterator();
  i.First();
  char* name = 0;
  string value;
  while(i.Next(name, value)) {
    serializeContentTypeParam(name, value, serialized);
  }
}

void HttpHeader::serializeHeaderField(const char* field_name, const string& field_value,
                                      string& serialized) const {
  serialized.append(field_name);
  serialized.append(": ");
  serialized.append(field_value);
  serialized.append(CRLF);
}

void HttpHeader::serialize(string& serialized) const {
  string _content_type;
  serializeContentType(_content_type);
  serializeHeaderField(CONTENT_TYPE, _content_type, serialized);
  Hash<string>::Iterator i = fields.getIterator();
  i.First();
  char* name = 0;
  string value;
  while(i.Next(name, value)) {
    serializeHeaderField(name, value, serialized);
  }
}

void HttpHeader::getEndBoundary(string& end_boundary) const {
  end_boundary.erase();
  if (boundary.empty()) {
    return;
  }
  end_boundary.append(boundary, 0, boundary.size() - CRLF_SIZE);
  end_boundary.append(BOUNDARY_START);
}

bool HttpHeader::isMultipart() const {
  return multipart;
}

void HttpHeader::addField(const char* name_buf, size_t name_size, const char* value_buf, size_t value_size) {
  string name(name_buf, name_size);
  string value(value_buf, value_size);
  fields.Insert(name.c_str(), value);   
}

void HttpHeader::addField(const char* name, string value) {
  fields.Insert(name, value);
}

size_t HttpHeader::getContentLength() {
  if (content_length) {
    return content_length;
  }
  const string * ptr = fields.GetPtr(CONTENT_LENGTH);
  if (ptr) {
    content_length = atoi((*ptr).c_str());
  }
  return content_length;
}

void HttpHeader::setBoundary(const string& _boundary) {
  boundary.erase();
  boundary += CRLF;
  boundary += BOUNDARY_START + _boundary + CRLF;
}

const string& HttpHeader::getBoundary() const {
  return boundary;
}

void HttpHeader::setCharset(const char* _charset) {
  charset = _charset;
}

void HttpHeader::addContentTypeParam(const char* param_name, string param_value) {
  content_type_params.Insert(param_name, param_value);
}

string HttpHeader::getHost() const {
  const string *host = fields.GetPtr(HOST);
  return host ? string(*host) : string("");
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
  string ser_header;
  serialize(ser_header);
  __trace2__("SerializedHeader:\n\'%s\'", ser_header.c_str());
}

void HttpHeader::clear() {
  charset.erase();
  content_type.erase();
  boundary.erase();
  content_length = 0;
  fields.Empty();
  content_type_params.Empty();
  multipart = false;
}

}//mms
}//transport
}//scag