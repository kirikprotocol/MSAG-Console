// Parser.cpp : Defines the entry point for the console application.
//
#include "HttpContext.h"
#include "HttpParser.h"
#include "HttpCommand.h"

char request[] = 
//"GET / HTTP/1.1\r\n"
//"GET /user/?login=aaa&passwd=aaa&url=%2Fuser%2Fbin%2Fstatus&x=41&y=8 HTTP/1.1\r\n"
"POST /79039005555_34567/love.nsk.ru/girls/top/12387.html HTTP/1.1\r\n"
//"GET /79039005555_34567/love.nsk.ru:80/girls/top/12387.html HTTP/1.1\r\n"
//"Empty-Value   :        \r\n"
//"  :empty key\r\n"
//"Content-Type: text/html; charset=utf-8  \r\n"
//"Content-Type:application/fastsoap\r\n"
//"Content-Type: text/html; charset=iso-8859-1 \r\n"
//"Connection: keep-alive\r\n"
"Content-Type: application/x-www-form-urlencoded\r\n"
"Content-Length:     57\r\n"
//"Server:distressed\r\n"
"\r\n"
//"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
"login=monkey&passwd=wrench&url=%2Fuser%2Fbin%2Fstatus&x=41&y=8";

char response[] = 
"HTTP/1.0 200 OK\r\n"
"Content-Type:application/fastsoap\r\n"
"Connection: keep-alive\r\n"
"Content-Length:10\r\n"
"Server:distressed\r\n"
"\r\n"
"1234567890";

int main(int argc, char* argv[])
{
  smsc::core::network::Socket         sock;
  scag::transport::http::HttpContext  ctx(&sock);
  scag::transport::http::StatusCode   code = scag::transport::http::CONTINUE;
  char                                *data = request;
  unsigned int                        all_size = sizeof(request) - 1;

  //ctx.req = new scag::transport::http::HttpRequest(ctx.getTransactionContext());
  //ctx.req->appendMessageContent(data, all_size);
  //ctx.req->appendMessageContent(request, all_size);

  ctx.action = scag::transport::http::READ_REQUEST;
  //ctx.action = scag::transport::http::READ_RESPONSE;

  while (code == scag::transport::http::CONTINUE) {
    char          buf[1024];
    unsigned int  len;
    unsigned int  unparsed_len = 0;

    unparsed_len = ctx.loadUnparsed(buf);

    len = 20 > all_size ? all_size: 20;
    memcpy(buf + unparsed_len, data, len);

    all_size -= len;
    data += len;
    unparsed_len += len;
    len = unparsed_len;
    code = scag::transport::http::HttpParser::parse(buf, unparsed_len, ctx);

    if (code == scag::transport::http::CONTINUE)
      ctx.saveUnparsed(buf + len - unparsed_len, unparsed_len);
  }

  return code;
}

