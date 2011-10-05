#include <stdlib.h>
#include <string.h>
#include <string>
#include "HttpContext.h"
#include "HttpCommand2.h"
#include "HttpParser.h"

namespace scag2 {
namespace transport {
namespace http {

const char HOST_FIELD[] = "Host";
const char CONTENT_TYPE_URL_ENCODED[] = "application/x-www-form-urlencoded";
const char CHARSET[] = "charset";
const char KEEP_ALIVE[] = "keep-alive";
const char COOKIE_FIELD[] = "Cookie";
const char SET_COOKIE_FIELD[] = "Set-Cookie";

/*
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
*/

//http_methods, method_table and METHOD_COUNT (table size) defined in HttpCommand2.h
//#define METHOD_COUNT (int)(sizeof(HttpRequest::method_table) / sizeof(HttpRequest::method_table[0]))

StatusCode HttpParser::parse(HttpContext& cx, bool last_data) {
	char *buf, *saved_buf, *local_buf;
	unsigned int len, local_len;

	StatusCode    rc = OK;
	HttpCommand   *command;

	switch (cx.action) {
	case READ_REQUEST:
		if (cx.command == NULL)
			cx.command = new HttpRequest(&cx, cx.getTransactionContext());
		break;
	case READ_RESPONSE:
		if (cx.command == NULL)
			cx.command = new HttpResponse(&cx, cx.getTransactionContext());
		break;
	default:
		return ERROR;
	}

	buf = cx.getUnparsed();
	len = cx.unparsedLength();
	buf += cx.parsePosition;
	len -= cx.parsePosition;

	local_len = len;
	saved_buf = buf;
	local_buf = buf;

  
	if (len == 0) {
//		smsc_log_debug(smsc::logger::Logger::getInstance("Http.Parser"), "return ok len=0");
		return OK;
	}
  
	command = cx.command;

	do {
		if (cx.flags == 0) {
			cx.parsePosition = 0;
			command->chunked = false;
			command->chunk_size = 0;
			command->setContentLength(0);
			rc = readLine(local_buf, local_len);
			if (rc != OK) {
				return rc;
			}

			if (local_len == 0)
				return ERROR;

			rc = parseFirstLine(saved_buf, local_len, cx);
			if (rc != OK) {
				return rc;
			}

			cx.parsePosition += static_cast<unsigned int>(local_buf - saved_buf);
			saved_buf = local_buf;
			local_len = len - static_cast<int>(local_buf - buf);

			cx.flags = 1;
		}

		while ((cx.flags == 1) && local_len) {
			rc = readLine(local_buf, local_len);
			if (rc != OK)
			  return rc;

			if (0 == local_len) {
				// empty line at the end of HTTP header
				cx.parsePosition += static_cast<unsigned int>(local_buf - saved_buf);
				saved_buf = local_buf;
				local_len = len - static_cast<int>(local_buf - buf);

				/*
				 *  cx.flags:
				 *  0 - default value
				 *  1 - after first header parsed
				 *  2 - after all headers parsed, waiting content
				 *  3 - after all headers parsed, chunked, waiting chunk size
				 *  4 - after all headers parsed, chunked, waiting chunk data
				 */
				cx.flags = (command->chunked) ? 3:2;
// RESPONSE may contain content without Content-Length header (RFC2068 ¤4.4)
				if ((cx.action == READ_REQUEST) && (command->contentLength == -1)) {
					switch (cx.getRequest().getMethod()) {
					case GET:
					case POST:
					case TRACE:
					case OPTIONS:
					case HEAD:
						return OK;
					default:
						break;
					}
				}
				break;
			}

			rc = parseHeaderFieldLine(saved_buf, local_len, cx, *command);
			if (rc != OK)
				return rc;

			cx.parsePosition += static_cast<unsigned int>(local_buf - saved_buf);
			saved_buf = local_buf;
			local_len = len - static_cast<int>(local_buf - buf);
		}

		if (rc != OK)
			break;

		if (cx.flags < 2) {	// rare case when end of line was exactly at buffer end
			return CONTINUE;
		}
		if (2 == cx.flags) {
			command->appendMessageContent(local_buf, local_len);
			cx.parsePosition += static_cast<unsigned int>(local_len);
			local_len = 0;

			if (last_data) {
				command->contentLength = command->content.GetPos();
				rc = OK;
			}
			else {
				rc = (static_cast<int>(command->content.GetPos()) < command->contentLength) ? CONTINUE : OK;
			}
			break;
		}

		while (local_len) {
			if (3 == cx.flags) {
				rc = readLine(local_buf, local_len);
				if (rc != OK) {
					return rc;
				}

				command->chunk_size = cx.chunks.add(std::string(saved_buf, local_len));
				if (0 == command->chunk_size) {
					command->contentLength = command->content.GetPos();
					rc = OK;
					break;
				}
				cx.flags = 4;
				cx.parsePosition += static_cast<unsigned int>(local_buf - saved_buf);
				local_len = len - static_cast<unsigned int>(local_buf - buf);
				saved_buf = local_buf;
			}
			if (4 == cx.flags) {
				local_len = command->appendChunkedMessageContent(local_buf, local_len);
				local_buf += local_len;
				if (0 == command->chunk_size) {
					rc = readLine(local_buf, local_len);	//read CRLF on chunk end
					if (rc != OK) {
						return rc;
					}
					if (local_len>0) {
						return ERROR;
					}

					cx.flags = 3;
				}
				cx.parsePosition += static_cast<unsigned int>(local_buf - saved_buf);
				local_len = len - static_cast<unsigned int>(local_buf - buf);
				saved_buf = local_buf;
				rc = CONTINUE;
			}
		}
		if (rc != OK)
			break;
		// analyse if command->content is already full
		if ((command->contentLength > 0) && (command->content.GetPos() >= size_t(command->contentLength))) {
			if (cx.action == READ_REQUEST) {
				const char *tmp = command->contentType.c_str();

				if ((cx.getRequest().getMethod() == POST) && !strcmp(tmp, CONTENT_TYPE_URL_ENCODED)) {
					char *pp = command->content.get();
					size_t ll = command->content.GetPos();

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
        cx.getResponse().setStatus(atoi(pos));
        if (!cx.getResponse().getStatus())
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
          if ((len >= HttpRequest::method_table[i].size) &&
              !compareNocaseN(buf, HttpRequest::method_table[i].name, HttpRequest::method_table[i].size)) {
            method_idx = i;
            break;
          }
        }

        if (method_idx == -1)
          return ERROR;

        const char *pos = findCharsN(buf, " \t", len);
        if (!pos)
          return ERROR;

        if (unsigned(pos - buf) != HttpRequest::method_table[method_idx].size)
          return ERROR;

        // we've got method
        cx.getRequest().setMethod(HttpRequest::method_table[method_idx].value);
//		smsc_log_debug(smsc::logger::Logger::getInstance("Http.Parser"), "setMethod method_idx=%d value=%d %s", method_idx, HttpRequest::method_table[method_idx].value, HttpRequest::method_table[method_idx].name);

        switch (HttpRequest::method_table[method_idx].value) {
          case GET:
          case POST:
            break;
          default:
            return OK;
        }

        pos++;
        const char *end = findCharsN(pos, " \t", len - static_cast<int>(pos - buf));
        std::string path;

        if (end) {
// - (end[-1] == '/' ? 1 : 0)            
          path.assign(pos, end - pos);
          end++;
          cx.getRequest().httpVersion.assign(end, len - (end - buf));
        } else
          return ERROR;

        if (HttpRequest::method_table[method_idx].value == GET) {
          pos = path.c_str();
          end = strchr(pos, '?');

          if (end) {
            parseQueryParameters(end + 1, cx.getRequest());
            path.erase(path.begin() + (end - pos), path.end());
          }
        }
        cx.getRequest().setSitePath(path);
      }
  default:
      break; // make compiler happy
  }
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
  while (--p >= buf && *p <= 32);

  if (p < buf)
    return ERROR; // no key

  key.assign(buf, p - buf + 1);
  pos++;

  while (pos < (buf + len) && *pos <= 32)
    pos++;

  value.assign(pos, len - (pos - buf));

//  for (unsigned int i = 0; i < key.length(); ++i)
//    key[i] = tolower(key[i]);

//  if (!strcasecmp(key.c_str(), KEEP_ALIVE))
//    return OK;

  if (!strcasecmp(key.c_str(), HOST_FIELD) && cx.action == READ_REQUEST) {
    return OK;
  }

  bool b;
  if (!(b = strcasecmp(key.c_str(), COOKIE_FIELD)) || !strcasecmp(key.c_str(), SET_COOKIE_FIELD))
    return parseCookie(value.c_str(), cmd, b);

  cmd.setHeaderField(key, value);

  if (!strcasecmp(key.c_str(), CONTENT_LENGTH_FIELD)) {
    cmd.setContentLength(atoi(value.c_str()));
    return OK;
  }
/*
 *   Connection: keep-alive
 *   Keep-Alive: 300
 */

  if ((key.compare(TRANSFER_ENCODING_FIELD) == 0) && (value.compare("chunked") == 0)) {
	  cmd.chunked = true;
	  return OK;
  }
  if ((key.compare(CONNECTION_FIELD) == 0) && (value.compare("close") == 0)) {
	  cmd.setCloseConnection(true);
	  return OK;
  }
  if ((key.compare(CONNECTION_FIELD) == 0) && (value.compare("keep-alive") == 0)) {
	  cmd.setCloseConnection(false);
	  return OK;
  }
  if (key.compare(KEEP_ALIVE_FIELD) == 0) {
	  cmd.setKeepAlive(HttpContext::fromString<int>(value));
	  cx.setTimeout(cmd.getKeepAlive());
	  return OK;
  }

  if (!strcasecmp(key.c_str(), CONTENT_TYPE_FIELD)) {
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
    const char *end = findCharsN(pos, " \t", static_cast<int>(value.size()));

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

//    for (unsigned int i = 0; i < key.length(); ++i)
//        key[i] = tolower(key[i]);

    mid++;

    if (end)
      value.assign(mid, end - mid);
    else
      value = mid;

    if(urlDecode(value) != OK)
        return ERROR;

    cx.setQueryParameter(key, value);

    start = end + 1;
  } while (end);

  return OK;
}

StatusCode HttpParser::parseCookie(const char *buf, HttpCommand& cmd, bool set)
{
    static const char* cookieParams[] = {"version", "path", "domain", "port", "max-age", "comment", "secure", "expires"};
    static const char cookieParamsCnt = sizeof(cookieParams)/sizeof(char*);

  const char  *start = buf, *mid, *end, *val;
  std::string key, value;
  Cookie *cur = &cmd.defCookie;

  do {
    end = strpbrk(start, ";");
    mid = strchr(start, '=');

    if (!mid || (mid && end && (mid > end)))
      return ERROR;

    while(start < mid && *start == ' ') start++;

    if(start >= mid || (*start == '$' && set))
        return ERROR;

    val = mid + 1;

    if(*start == '$')
        cur->setParam(start + 1, static_cast<uint32_t>(mid - start - 1), val, end ? static_cast<uint32_t>(end - val) : static_cast<uint32_t>(strlen(val)));
    else
    {
        int i = 0;
        while(i < cookieParamsCnt && strncasecmp(cookieParams[i], start, mid - start)) i++;
        if(i < cookieParamsCnt)
            cur->setParam(start, static_cast<uint32_t>(mid - start), val, end ? static_cast<uint32_t>(end - val) : static_cast<uint32_t>(strlen(val)));
        else
        {
            cur = new Cookie;
            cur->name.assign(start, mid - start);

//            for (unsigned int i = 0; i < cur->name.length(); ++i)
//                cur->name[i] = tolower(cur->name[i]);

            if(end)
                cur->value.assign(val, end - val);
            else
                cur->value.assign(val);
            cmd.cookies.Insert(cur->name.c_str(), cur);
        }
    }

    start = end + 1;
  } while (end && start && *start);

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
