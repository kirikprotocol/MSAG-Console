#ifndef SCAG_TRANSPORT_HTTP_BASE_PARSER
#define SCAG_TRANSPORT_HTTP_BASE_PARSER

#define SESSION_ID_ENABLED

#ifdef _WIN32
#ifdef ERROR
#undef ERROR
#endif
#endif

#define HTTP_HOST "host"
#define HTTP_CONTENT_LENGTH "content-length"
#define HTTP_CONTENT_TYPE "content-type"
#define HTTP_CONTENT_TYPE_URL_ENCODED "application/x-www-form-urlencoded"

namespace scag2 {
namespace transport {
namespace http {

class HttpContext;
class HttpCommand;
class HttpRequest;

enum StatusCode { 
  OK,
  CONTINUE,
  ERROR
};

class HttpParser {
public:
//  static StatusCode parse(char* buf, unsigned int& len, HttpContext& cx);
  static StatusCode parse(HttpContext& cx);

  static void urlEncode(std::string &s);

  static StatusCode urlDecode(std::string &s);

private:
  static StatusCode readLine(char*& buf, unsigned int& len);

  static StatusCode parseFirstLine(char *buf, unsigned int len, HttpContext& cx);

  static StatusCode parseHeaderFieldLine(char *buf, unsigned int len, HttpContext &cx, HttpCommand& cmd);

  static StatusCode parseQueryParameters(const char *buf, HttpRequest& cx);

  static StatusCode parseCookie(const char *buf, HttpCommand& cmd, bool set);

  static int compareNocaseN(const char *src, const char *dst, int count);

  static const char *findCharsN(const char *src, const char *dst, int count);

  static const char *stringStrN(const char *str, const char *search);
};

}}}

#endif // SCAG_TRANSPORT_HTTP_PARSER
