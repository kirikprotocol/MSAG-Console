#ifndef __SCAG_TRANSPORT_MMS_UTIL_H__
#define __SCAG_TRANSPORT_MMS_UTIL_H__

#include <string.h>
#include <string>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>

#include "logger/Logger.h"
#include "util/debug.h"

//#include "scag/transport/mms/XMLHandlers.h"
//#include "XMLHandlers.h"

namespace scag {
namespace transport {
namespace mms {

XERCES_CPP_NAMESPACE_USE

class XStr {
public:
  XStr(const char* const to_transcode) {
    unicode_form = XMLString::transcode(to_transcode);
  }
  ~XStr() {
    XMLString::release(&unicode_form);
  } 
  const XMLCh* unicodeForm() const {
    return unicode_form;
  }

private:
  XMLCh* unicode_form;
};

class StrX {
public:
  StrX(const XMLCh* const to_transcode) {
    local_form = XMLString::transcode(to_transcode);
  }
  ~StrX() {
    XMLString::release(&local_form);
  } 
  const char* localForm() const {
    return local_form;
  }

private:
  char* local_form;
};

namespace xml {
  extern const char* TRUE;
  extern const char* FALSE;
  extern const char* MM7_TRANSACTION_ID;
  extern const char* TRANSACTION_ID;
  extern const char* NUMBER;
  extern const char* SHORT_CODE;
  extern const char* RFC2822;
  extern const char* CONTENT;
  extern const char* REPLY_CHARGING;
  extern const char* REPLY_CHARGING_SIZE;
  extern const char* REPLY_DEADLINE;
  extern const char* DISPLAY_ONLY;
  extern const char* ADDRESS_CODING;
  extern const char* ENCRYPTED;
  extern const char* OBFUSCATED;
  extern const char* TO;
  extern const char* CC;
  extern const char* BCC;
  extern const char* SENDER_ADDRESS;
  extern const char* SENDER;
  extern const char* DATE_TIME;
  extern const char* USER_AGENT;
  extern const char* SEQUENCE;
  extern const char* UA_TIME_STAMP;
  extern const char* TIME_STAMP;
  extern const char* UA_PROF;
  extern const char* UA_CAPABILITIES;
  extern const char* CANCEL;
  extern const char* EXTENDED_CANCEL;
  extern const char* REPLACE;
  extern const char* EXTENDED_REPLACE;
  extern const char* READ_REPLY;
  extern const char* DELIVERY_REPORT;
  extern const char* RECIPIENT;
  extern const char* RECIPIENTS;
  extern const char* SERVICE_CODE;
  extern const char* LINKED_ID;
  extern const char* MESSAGE_ID;
  extern const char* CANCEL_ID;
  extern const char* REPLACE_ID;
  extern const char* MESSAGE_CLASS;
  extern const char* EARLIEST_DELIVERY_TIME;
  extern const char* EXPIRY_DATE;
  extern const char* DELIVERY_REPORT_TAG;
  extern const char* READ_REPLY_TAG;
  extern const char* PRIORITY;
  extern const char* SUBJECT;
  extern const char* CHARGED_PARTY;
  extern const char* CHARGED_PARTY_ID;
  extern const char* REPLAY_CHARGING_ID;
  extern const char* DISTRIBUTION_INDICATOR;
  extern const char* DELIVERY_CONDITION;
  extern const char* APPLIC_ID;
  extern const char* REPLY_APPLIC_ID;
  extern const char* AUX_APPLIC_INFO;
  extern const char* CONTENT_CLASS;
  extern const char* DRM_CONTENT;
  extern const char* HREF;
  extern const char* ALLOW_ADAPTATIONS;
  extern const char* PREVIOUSLY_SENT_BY;
  extern const char* PREVIOUSLY_SENT_DATE;
  extern const char* SENDER_SPI;
  extern const char* RECIPIENT_SPI;
  extern const char* MMS_RS_ID;
  extern const char* MM_STATUS;
  extern const char* MM_STATUS_EXTENSION;
  extern const char* STATUS_TEXT;
  extern const char* DATE;
  extern const char* STATUS;
  extern const char* STATUS_CODE;
  extern const char* FAULT;
  extern const char* FAULT_CODE;
  extern const char* FAULT_STRING;
  extern const char* DETAIL;
  
  extern const char* MUST_UNDERSTAND;
  extern const char* ENCODING_STYLE;
  extern const char* ACTOR;
  extern const char* ENV_PREFIX;
  extern const char* MM7_PREFIX;
  extern const char* XMLNS;
  extern const char* XMLNS_MM7;

  extern const char* ENVELOPE;
  extern const char* HEADER;
  extern const char* BODY;
  extern const char* ENVELOPE_URI;
  extern const char* MM7_URI;
  extern const char* SCHEMA_LOCATION;
  
  extern const char* SENDER_ID;
  extern const char* MM7_VERSION;
  extern const char* VAS_ID;
  extern const char* VASP_ID;
  extern const char* MMS_RELAY_SERVER_ID;
  
  extern const char* UTF_16;
  extern const char* UTF_8;

  using std::string;

  inline void addTextNode(DOMDocument* doc, DOMElement* parent,
                           const char* name, const string& value) {
    DOMElement* node = doc->createElement(XStr(name).unicodeForm());
    parent->appendChild(node);
    DOMText* node_value = doc->createTextNode(XStr(value.c_str()).unicodeForm());
    node->appendChild(node_value);
  };

  inline bool replaceXmlnsValue(string& soap_envelope) {
    size_t mm7_pos = soap_envelope.find(xml::XMLNS_MM7);
    if (mm7_pos == string::npos) {
      return false;
    }
    size_t ns_start = soap_envelope.find_first_of("\"\'", mm7_pos);
    if (ns_start == string::npos) {
      return false;
    }
    size_t ns_end = soap_envelope.find_first_of("\"\'", ++ns_start);
    if (ns_end == string::npos) {
      return false;
    }
    soap_envelope.replace(ns_start, ns_end - ns_start, MM7_URI);
    size_t ns_pos = soap_envelope.find(xml::XMLNS, ns_end);
    if (ns_pos == string::npos) {
      return false;
    }
    ns_start = soap_envelope.find_first_of("\"\'", ns_pos);
    if (ns_start == string::npos) {
      return false;
    }
    ns_end = soap_envelope.find_first_of("\"\'", ++ns_start);
    if (ns_end == string::npos) {
      return false;
    }
    soap_envelope.replace(ns_start, ns_end - ns_start, MM7_URI);
    return true;
  }
}

static const size_t MM7_TIME_BUFFER_SIZE = 26;

static inline bool stringToBool(const char* str, bool def = false) {
  if (!str) {
    return def; 
  }
  if ((strcmp(str, "1") == 0) || (strcmp(str, scag::transport::mms::xml::TRUE) == 0)) {
    return true;
  }
  if ((strcmp(str, "0") == 0) || (strcmp(str, scag::transport::mms::xml::FALSE) == 0)) {
    return false;
  }
  return def;
};
static inline bool cTime2Mm7Time(time_t c_time, char* mms_time) {
  if (!mms_time || !c_time) {
    return false;
  }
  struct tm timev = *gmtime(&c_time);
  int tz_min = abs(timezone / 60);
  int tz_hour = tz_min / 60;
  tz_min = tz_min % 60;
  char sign = 0;
  if (timezone > 0 ) {
    sign = '-';
  } else {
    sign = '+';
  }
  timev.tm_mon += 1;
  timev.tm_year += 1900;
  __ret0_if_fail__(timev.tm_year > 1900 && timev.tm_year <= 2100);
  __ret0_if_fail__(timev.tm_mon >= 1 && timev.tm_mon <= 12);
  __ret0_if_fail__(timev.tm_mday >= 1 && timev.tm_mday <= 31);
  __ret0_if_fail__(timev.tm_hour >= 0 && timev.tm_hour <= 23);
  __ret0_if_fail__(timev.tm_min >= 0 && timev.tm_min <= 59);
  __ret0_if_fail__(timev.tm_sec >= 0 && timev.tm_sec <= 59);
  __ret0_if_fail__(tz_hour >= 0 && tz_hour <= 14);
  int writen = snprintf(mms_time, MM7_TIME_BUFFER_SIZE, 
                        "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
                         timev.tm_year,
                         timev.tm_mon,
                         timev.tm_mday,
                         timev.tm_hour,
                         timev.tm_min,
                         timev.tm_sec,
			             sign,
                         tz_hour,
                         tz_min);
  return true;
}
static inline time_t mm7Time2CTime(const char* mms_time) {
  time_t result_time;
  if (!mms_time) {
    return result_time;
  }
  size_t time_size = strlen(mms_time);
  int sign = 1;
  if (*mms_time == '-') {
    sign = -1;
    ++mms_time;
  }
  if (*mms_time == 'P') {
    ++mms_time;
    int n = 0;
    bool t = false;
    int number = 0;
    uint64_t relative_time = 0;
    while (*mms_time) {
      if (isdigit(*mms_time)) {
        ++n;
      }
      if (isalpha(*mms_time)) {
        char* number_buf = new char[n + 1];
	std::strncpy(number_buf, mms_time - n, n);
	number_buf[n] = 0;
	number = atoi(number_buf);
	//__trace2__("%s %d \'%s\' %d", mms_time - n, number, number_buf, n);
	delete[] number_buf;
	n = 0;
      }
      switch (*mms_time) {
        case 'T' : 
	     t = true;
	     break;
        case 'Y' : 
	     relative_time += (365.25 * 24 * 3600 * number);
	     break;
        case 'M' : 
	      if (t) {
	        relative_time += (60 * number);
	      } else {
	        relative_time += (30 * 24 * 3600 * number);
	      }
	      break;
        case 'D' : 
	     relative_time += (24 * 3600 * number);
	     break;
        case 'H' : 
	     relative_time += (3600 * number);
	     break;
	case 'S' :
	     relative_time += number;
	     break;
	default : break;
      }
      ++mms_time;
    }
    result_time = time(0);
    return result_time + relative_time * sign;
  }
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;
  int n = sscanf(mms_time, "%04d-%02d-%02dT%02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);
  //__trace2__("n = %d %d %d %d %d %d", year, month, day, hour, min, sec);
  mms_time = mms_time + 19;
  struct tm timev;
  timev.tm_isdst = 0;
  timev.tm_year = year - 1900;
  timev.tm_mon = month - 1;
  timev.tm_mday = day;
  timev.tm_hour = hour;
  timev.tm_min = min;
  timev.tm_sec = sec;
  result_time = mktime(&timev);
  result_time -= timezone;
  return result_time;
  //if (!mms_time || *mms_time == 0) {
    //return result_time;
  //}
  //if (*mms_time == '-' || *mms_time == '+') {
    //int h = 0;
    //int m = 0;
    //n = sscanf(mms_time, "%03d:%02d", &h, &m);
    //result_time += (h * 60 + (h / abs(h) * min)) * 60;
    //return result_time;
  //}
}


}//mms
}//transport
}//scag

#endif

