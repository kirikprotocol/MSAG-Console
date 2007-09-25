#ifndef __SCAG_MMS_UTIL_H__
#define __SCAG_MMS_UTIL_H__

#include <string.h>
#include <string>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>

#include "logger/Logger.h"

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
  static const char* TRUE                   = "true";
  static const char* FALSE                  = "false";
  static const char* MM7_TRANSACTION_ID     = "mm7:TransactionID";
  static const char* TRANSACTION_ID         = "TransactionID";
  static const char* NUMBER                 = "Number";
  static const char* SHORT_CODE             = "ShortCode";
  static const char* RFC2822                = "RFC2822Address";
  static const char* CONTENT                = "Content";
  static const char* REPLY_CHARGING         = "ReplyCharging";
  static const char* REPLY_CHARGING_SIZE    = "replyChargingSize";
  static const char* REPLY_DEADLINE         = "replyDeadline";
  static const char* DISPLAY_ONLY           = "displayOnly";
  static const char* ADDRESS_CODING         = "addressCoding";
  static const char* ENCRYPTED              = "encrypted";
  static const char* OBFUSCATED             = "obfuscated";
  static const char* TO                     = "To";
  static const char* CC                     = "Cc";
  static const char* BCC                    = "Bcc";
  static const char* SENDER_ADDRESS         = "SenderAddress";
  static const char* SENDER                 = "Sender";
  static const char* DATE_TIME              = "DateTime";
  static const char* USER_AGENT             = "UserAgent";
  static const char* SEQUENCE               = "sequence";
  static const char* UA_TIME_STAMP          = "UATimeStamp";
  static const char* TIME_STAMP             = "TimeStamp";
  static const char* UA_PROF                = "UAProf";
  static const char* UA_CAPABILITIES        = "UACapabilities";
  static const char* CANCEL                 = "CancelReq";
  static const char* EXTENDED_CANCEL        = "extendedCancelReq";
  static const char* REPLACE                = "ReplaceReq";
  static const char* EXTENDED_REPLACE       = "extendedReplaceReq";
  static const char* READ_REPLY             = "ReadReplyReq";
  static const char* DELIVERY_REPORT        = "DeliveryReportReq";
  static const char* RECIPIENT              = "Recipient";
  static const char* RECIPIENTS             = "Recipients";
  static const char* SERVICE_CODE           = "ServiceCode";
  static const char* LINKED_ID              = "LinkedID";
  static const char* MESSAGE_ID             = "MessageID";
  static const char* CANCEL_ID              = "CancelID";
  static const char* REPLACE_ID             = "ReplaceID";
  static const char* MESSAGE_CLASS          = "MessageClass";
  static const char* EARLIEST_DELIVERY_TIME = "EarlestDeliveryTime";
  static const char* EXPIRY_DATE            = "ExpiryDate";
  static const char* DELIVERY_REPORT_TAG    = "DeliveryReport";
  static const char* READ_REPLY_TAG         = "ReadReply";
  static const char* PRIORITY               = "Priority";
  static const char* SUBJECT                = "Subject";
  static const char* CHARGED_PARTY          = "ChargedParty";
  static const char* CHARGED_PARTY_ID       = "ChargedPartyID";
  static const char* REPLAY_CHARGING_ID     = "ReplayChargingID";
  static const char* DISTRIBUTION_INDICATOR = "DistributionIndicator";
  static const char* DELIVERY_CONDITION     = "DeliveryCondition";
  static const char* APPLIC_ID              = "ApplicID";
  static const char* REPLY_APPLIC_ID        = "ReplyApplicID";
  static const char* AUX_APPLIC_INFO        = "AuxApplicInfo";
  static const char* CONTENT_CLASS          = "ContentClass";
  static const char* DRM_CONTENT            = "DRMContent";
  static const char* HREF                   = "href";
  static const char* ALLOW_ADAPTATIONS      = "allowAdaptations";
  static const char* PREVIOUSLY_SENT_BY     = "Previouslysentby";
  static const char* PREVIOUSLY_SENT_DATE   = "Previouslysentdateandtime";
  static const char* SENDER_SPI             = "SenderSPI";
  static const char* RECIPIENT_SPI          = "RecipientSPI";
  static const char* MMS_RS_ID              = "MMSRelayServerID";
  static const char* MM_STATUS              = "MMStatus";
  static const char* MM_STATUS_EXTENSION    = "MMStatusExtension";
  static const char* STATUS_TEXT            = "StatusText";
  static const char* DATE                   = "Date";
  static const char* STATUS                 = "Status";
  static const char* STATUS_CODE            = "StatusCode";
  static const char* FAULT                  = "Fault";
  static const char* FAULT_CODE             = "faultcode";
  static const char* FAULT_STRING           = "faultstring";
  static const char* DETAIL                 = "detail";
  
  static const char* MUST_UNDERSTAND        = "env:mustUnderstand";
  static const char* ENCODING_STYLE         = "env:encodingStyle";
  static const char* ACTOR                  = "env:actor";
  static const char* ENV_PREFIX             = "env";
  static const char* MM7_PREFIX             = "mm7";
  static const char* XMLNS                  = "xmlns";
  static const char* XMLNS_MM7              = "xmlns:mm7";

  static const char* ENVELOPE               = "Envelope";
  static const char* HEADER                 = "Header";
  static const char* BODY                   = "Body";
  static const char* ENVELOPE_URI           = "http://schemas.xmlsoap.org/soap/envelope/";
  static const char* MM7_URI                = "http://www.3gpp.org/ftp/Specs/archive/23_series/23.140/schema/REL-6-MM7-1-4";
  static const char* SCHEMA_LOCATION        = "http://www.3gpp.org/ftp/Specs/archive/23_series/23.140/schema/REL-6-MM7-1-4 ./REL-6-MM7-1-4.xsd";
  
  static const char* SENDER_ID             = "SenderIdentification";
  static const char* MM7_VERSION           = "MM7Version";
  static const char* VAS_ID                = "VASID";
  static const char* VASP_ID               = "VASPID";
  static const char* MMS_RELAY_SERVER_ID   = "MMSRelayServerID";
  
  static const char* UTF_16                = "UTF-16";
  static const char* UTF_8                 = "UTF-8";

  using std::string;

  inline void addTextNode(DOMDocument* doc, DOMElement* parent, const char* name, std::string value) {
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
  size_t time_size = std::strlen(mms_time);
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

