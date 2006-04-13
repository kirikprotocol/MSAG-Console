#include <stdlib.h>
#include <string.h>
#include <string>
#include "HttpContext.h"
#include "HttpCommand.h"
#include "HttpParser.h"

namespace scag { namespace transport { namespace http
{

const char HOST_FIELD[] = "host";
const char CONTENT_TYPE_URL_ENCODED[] = "application/x-www-form-urlencoded";
const char CHARSET[] = "charset";
const char KEEP_ALIVE[] = "keep-alive";

struct http_methods {
  const char   *name; 
  unsigned int  size;
  HttpMethod    value;
};

static http_methods method_table[] = {
  {"GET", 3, GET},
  {"POST", 4, POST},
  {"PUT", 3, PUT},
  {"HEAD", 4, HEAD},
  {"TRACE", 5, TRACE},
  {"DELETE", 6, DELETE},
  {"CONNECT", 7, CONNECT},
  {"OPTIONS", 7, OPTIONS}
};

#define METHOD_COUNT (int)(sizeof(method_table) / sizeof(method_table[0]))

StatusCode HttpParser::parse(char* buf, unsigned int& len, HttpContext& cx)
{
  char          *saved_buf = buf, *local_buf = buf;
  unsigned int  local_len = len;
  StatusCode    rc = OK;
  HttpCommand   *command;

  if (len == 0)
    return OK;

  switch (cx.action) {
    case READ_REQUEST:
      if (cx.command == NULL)
        cx.command = new HttpRequest(cx.getTransactionContext());
      break;
    case READ_RESPONSE:
      if (cx.command == NULL)
        cx.command = new HttpResponse(cx.getTransactionContext());
      break;
    default:
      return ERROR;
  }
  command = cx.command;

  do {
    if (cx.flags == 0) {
      rc = readLine(local_buf, local_len);
      if (rc != OK)
        break;

      if (local_len == 0)
        return ERROR;

      rc = parseFirstLine(saved_buf, local_len, cx);
      if (rc != OK)
        return rc;

      saved_buf = local_buf;
      local_len = len - (local_buf - buf);

      cx.flags = 1;
    }

    while ((cx.flags == 1) && local_len) {
      rc = readLine(local_buf, local_len);
      if (rc != OK)
        break;

      if ((rc == OK) && (local_len == 0)) {
        // empty line at the end of HTTP header
        cx.flags = 2;
        local_len = len - (local_buf - buf);

        if (command->contentLength == 0)
          return OK;

        if ((cx.action == READ_REQUEST) && (command->contentLength == -1)) {
          switch (cx.getRequest().getMethod()) {
            case GET:
            case TRACE:
            case OPTIONS:
            case HEAD:
              return OK;
          }
        }

        break;
      }

      rc = parseHeaderFieldLine(saved_buf, local_len, cx, *command);
      if (rc != OK)
        return rc;

      saved_buf = local_buf;
      local_len = len - (local_buf - buf);
    }

    if (rc != OK)
      break;

    if (cx.flags != 2) {
      // rare case when end of line was exactly at buffer end
      len = 0;
      return CONTINUE;
    }

    command->appendMessageContent(local_buf, local_len);
    len = 0;

    if ((command->contentLength > 0) && (command->content.GetPos() >= command->contentLength)) {
      if (cx.action == READ_REQUEST) {
        const char *tmp = command->contentType.c_str();

        if ((cx.getRequest().getMethod() == POST) && !strcmp(tmp, CONTENT_TYPE_URL_ENCODED)) {
          char *pp = command->content.get();
          int ll = command->content.GetPos();
          
          while (ll && (pp[ll-1] == '\r' || pp[ll-1] == '\n'))
            ll--;            
        
          std::string params(pp, ll);
          parseQueryParameters(params.c_str(), cx.getRequest());
        }
      }

      return OK;
    }

    return CONTINUE;
  } while (false);

  if (rc == CONTINUE) {
    len = len - (saved_buf - buf);
  }

  return rc;
}

StatusCode HttpParser::readLine(char*& buf, unsigned int& len)
{
  int   count = len;

  len = 0;
  while (count--) {
    char ch = *buf++;

    if (ch == '\0')
      return ERROR;

    if (ch == '\r')
      continue;

    if (ch == '\n')
      return OK;

    len++;
  }

  return CONTINUE;
}

StatusCode HttpParser::parseFirstLine(char *buf, unsigned int len, HttpContext& cx)
{
  switch (cx.action) {
    case READ_RESPONSE:
      {
        if ((len < 5) || compareNocaseN(buf, "HTTP/", 5))
          return ERROR;

        const char *pos = findCharsN(buf, " \t", len);
        if (!pos)
          return ERROR;

        cx.getResponse().httpVersion.assign(buf, pos - buf);

        pos++;
        cx.getResponse().status = atoi(pos);
        if (!cx.getResponse().status)
          return ERROR;

        while (isdigit(*pos))
            pos++;
        while (*pos == ' ')
            pos++;

        cx.getResponse().statusLine.assign(pos, len - (pos - buf));
      }
      break;
    case READ_REQUEST:
      {
        int   method_idx = -1;

        for (int i = 0; i < METHOD_COUNT; ++i) {
          if ((len >= method_table[i].size) && 
              !compareNocaseN(buf, method_table[i].name, method_table[i].size)) {
            method_idx = i;
            break;
          }
        }

        if (method_idx == -1)
          return ERROR;

        const char *pos = findCharsN(buf, " \t", len);
        if (!pos)
          return ERROR;

        if ((pos - buf) != method_table[method_idx].size)
          return ERROR;

        // we've got method
        cx.getRequest().setMethod(method_table[method_idx].value);

        switch (method_table[method_idx].value) {
          case GET:
          case POST:
            break;
          default:
            return OK;
        }

        pos++;
        const char *end = findCharsN(pos, " \t", len - (pos - buf));
        std::string path;

        if (end) {
          path.assign(pos, end - pos - (end[-1] == '/' ? 1 : 0));
          end++;
          cx.getRequest().httpVersion.assign(end, len - (end - buf));
        } else
          return ERROR;

        if (method_table[method_idx].value == GET) {
          pos = path.c_str();
          end = strchr(pos, '?');

          if (end) {
            parseQueryParameters(end + 1, cx.getRequest());
            path.erase(path.begin() + (end - pos), path.end());
          }
        }
        return parsePath(path, cx.getRequest());
      }
  }
  return OK;
}

StatusCode HttpParser::parsePath(std::string &path, HttpRequest& cx)
{
  const char    *pos = path.c_str(), *mid, *end;
  std::string   str;
  unsigned int len;
   
  end = strchr(pos, '/');
  if (!end)
    return ERROR;

#ifdef SESSION_ID_ENABLED
  if (end == pos) {
    pos++;
    end = strchr(pos, '/');
    if (!end)
      return ERROR;
  }

  mid = pos;
  while (mid <= end && isalnum(*mid))
    mid++;

  len = mid - pos;
  if (!(mid <= end && *mid == '_' && 1 <= len && len <= 20))
    return ERROR;

  str.assign(pos, len);
  cx.setAbonent(str);
  mid++;
  len = end - mid;
  str.assign(mid, len);
  
  pos = str.c_str();
  while (isdigit(*pos))
    pos++;
  if (*pos || !(1 <= len && len <= 5))
    return ERROR;
  
  len = atoi(str.c_str());
  if (len > USHRT_MAX)
    return ERROR;
  
  cx.setUSR(len);
#endif
 
  pos = end + 1;
  end = strchr(pos, '/');
  if (!end)
    return ERROR;

  mid = strchr(pos, ':');
  if (mid && (mid < end)) {
    str.assign(pos, mid - pos);
    cx.setSite(str);
    mid++;
    str.assign(mid, end - mid);
    cx.sitePort = atoi(str.c_str());
  } else {
    str.assign(pos, end - pos);
    cx.setSite(str);
  }

  pos = end;
  if (*pos)
    cx.sitePath.assign(pos);

  return OK;
}

StatusCode HttpParser::parseHeaderFieldLine(char *buf, unsigned int len,
    HttpContext &cx, HttpCommand& cmd)
{
  std::string key, value;
  const char  *pos = findCharsN(buf, ":", len);

  if (!pos)
    return ERROR;

  const char  *p = pos;
  while (--p > buf) {
    if (*p > 32)
      break;
  }

  if (p == buf)
    return ERROR; // no key

  key.assign(buf, p - buf + 1);
  pos++;

  while (pos < (buf + len) && *pos <= 32)
    pos++;

  value.assign(pos, len - (pos - buf));

  for (unsigned int i = 0; i < key.length(); ++i)
    key[i] = tolower(key[i]);

  if (!strcmp(key.c_str(), KEEP_ALIVE))
    return OK;

  if (!strcmp(key.c_str(), HOST_FIELD) && cx.action == READ_REQUEST) {
    cmd.setHeaderField(key, cx.getRequest().getSite());   
    return OK;
  }

  cmd.setHeaderField(key, value);

  if (!strcmp(key.c_str(), CONTENT_LENGTH_FIELD)) {
    cmd.setContentLength(atoi(value.c_str()));
    return OK;
  }
 
  if (!strcmp(key.c_str(), CONTENT_TYPE_FIELD)) {
    pos = stringStrN(value.c_str(), CHARSET);

    if (pos) {
      pos = strchr(pos, '=');
      if (!pos)
        return OK;

      pos++;
      while (*pos && *pos <= 32)
        pos++;

      if (!*pos)
        return OK;

      const char *end = strpbrk(pos, " \t");
      if (!end)
        cmd.charset = pos;
      else
        cmd.charset.assign(pos, end - pos);

      for (unsigned int i = 0; i < cmd.charset.length(); ++i)
        cmd.charset[i] = tolower(cmd.charset[i]);
    }

    pos = value.data();
    const char *end = findCharsN(pos, " \t", value.size());

    if (!end)
      cmd.contentType = value;
    else
      cmd.contentType.assign(pos, end - pos);

    for (unsigned int i = 0; i < cmd.contentType.length(); ++i)
      cmd.contentType[i] = tolower(cmd.contentType[i]);
  }

  return OK;
}

StatusCode HttpParser::parseQueryParameters(const char *buf, HttpRequest& cx)
{
  const char  *start = buf, *mid, *end;
  std::string key, value;

  cx.siteQuery.assign(buf, strlen(buf));

  do {
    end = strchr(start, '&');
    mid = strchr(start, '=');

    if (!mid || (mid && end && (mid > end)))
      return ERROR;

    key.assign(start, mid - start);
    mid++;

    if (end)
      value.assign(mid, end - mid);
    else
      value = mid;

    cx.setQueryParameter(key, value);

    start = end + 1;
  } while (end);

  return OK;
}

int HttpParser::compareNocaseN(const char *src, const char *dst, int count)
{
  int   f, l;

  if (count) {
    do {
      f = tolower(*(dst++));
      l = tolower(*(src++));
    } while (--count && f && (f == l));

    return (f - l);
  }
  return 0;
}

const char *HttpParser::findCharsN(const char *src, const char *dst, int count)
{
  size_t size = strlen(dst);

  if (count && size)
    do {
      for (size_t i = 0; i < size; ++i) {
        if (*src == dst[i])
          return src;
      }
      src++;
    } while (--count);

  return NULL;
}

const char *HttpParser::stringStrN(const char *str, const char *search)
{
  if (str && search)
  {
    if (*search == 0)
      return str;

    while (*str)
    {
      const char *s1 = search;
      const char *s2 = str;
      // see if substring characters match
      while (tolower(*(s1++)) == tolower(*(s2++)))
      {
        if (*s1 == 0)
          return str;
      }
      ++str;
    }
  }

  return NULL;
}

StatusCode HttpParser::urlDecode(std::string &s) 
{
  const char  *pos;
  char        *result, *respos; 
  char        buf[] = "0x00";

  respos = result = new char[s.length() + 1];

  for (pos = s.c_str(); *pos != '\0'; pos++) {
    if (*pos == '%') {
      if (!isxdigit(pos[1]) || !isxdigit(pos[2])) {
          delete[] result;
          return ERROR;
        }

        buf[2] = *++pos; 
        buf[3] = *++pos; 

        *respos++ = (char) strtol(buf, NULL, 16);
    } else
      *respos++ = *pos;
  }

  *respos = '\0';
  s = result;
  delete[] result;

  return OK;
}

// ABNF definitions derived from RFC3986, except with "/" removed from
// gen-delims since it's special:

#define GD (1) // gen-delims    = ":" / "?" / "#" / "[" / "]" / "@"
#define SD (1) // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                               // "*" / "+" / "," / ";" / "=" 
#define SL (0) // forward-slash = "/" 
#define UN (0) // unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~" 
#define OT (1) // others

// Lookup table for percent-encoding logic: value is non-zero if
// character should be percent-encoded.
static const unsigned char uri_chars[128] = {
  // 0xXX    x0      x2      x4      x6      x8      xA      xC      xE  
  /*   0x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
  /*   1x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
  /*   2x */ OT, SD, OT, GD, SD, OT, SD, SD, SD, SD, SD, SD, SD, UN, UN, SL,
  /*   3x */ UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, GD, SD, OT, SD, OT, GD,
  /*   4x */ GD, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN,
  /*   5x */ UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, GD, OT, GD, OT, UN,
  /*   6x */ OT, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN,
  /*   7x */ UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, UN, OT, OT, OT, UN, OT 
};

#undef SD
#undef GD
#undef SL
#undef UN
#undef OT

void HttpParser::urlEncode(std::string &s)
{
  const unsigned char *src;
  char                *result, *dst;
  size_t              count = 0;

  for (src = (const unsigned char *) s.c_str(); *src != '\0'; src++) {
    if ((*src) > 127 || uri_chars[(*src)])
      count++;
  }

  if (!count)
    return;

  dst = result = new char[s.length() + 2 * count + 1];

  for (src = (const unsigned char *) s.c_str(); *src != '\0'; src++) {
    if ((*src) > 127 || uri_chars[(*src)]) {
      // Escape it - %<hex><hex> 
      sprintf(dst, "%%%02x", (unsigned char) *src);
      dst += 3;
    } else
      *dst++ = *src;
  }

  *dst = '\0';
  s = result;
  delete[] result;
}

}}}
