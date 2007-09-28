#include "util.h"

namespace scag { namespace transport { namespace mms {

namespace xml {

  const char* TRUE                   = "true";
  const char* FALSE                  = "false";
  const char* MM7_TRANSACTION_ID     = "mm7:TransactionID";
  const char* TRANSACTION_ID         = "TransactionID";
  const char* NUMBER                 = "Number";
  const char* SHORT_CODE             = "ShortCode";
  const char* RFC2822                = "RFC2822Address";
  const char* CONTENT                = "Content";
  const char* REPLY_CHARGING         = "ReplyCharging";
  const char* REPLY_CHARGING_SIZE    = "replyChargingSize";
  const char* REPLY_DEADLINE         = "replyDeadline";
  const char* DISPLAY_ONLY           = "displayOnly";
  const char* ADDRESS_CODING         = "addressCoding";
  const char* ENCRYPTED              = "encrypted";
  const char* OBFUSCATED             = "obfuscated";
  const char* TO                     = "To";
  const char* CC                     = "Cc";
  const char* BCC                    = "Bcc";
  const char* SENDER_ADDRESS         = "SenderAddress";
  const char* SENDER                 = "Sender";
  const char* DATE_TIME              = "DateTime";
  const char* USER_AGENT             = "UserAgent";
  const char* SEQUENCE               = "sequence";
  const char* UA_TIME_STAMP          = "UATimeStamp";
  const char* TIME_STAMP             = "TimeStamp";
  const char* UA_PROF                = "UAProf";
  const char* UA_CAPABILITIES        = "UACapabilities";
  const char* CANCEL                 = "CancelReq";
  const char* EXTENDED_CANCEL        = "extendedCancelReq";
  const char* REPLACE                = "ReplaceReq";
  const char* EXTENDED_REPLACE       = "extendedReplaceReq";
  const char* READ_REPLY             = "ReadReplyReq";
  const char* DELIVERY_REPORT        = "DeliveryReportReq";
  const char* RECIPIENT              = "Recipient";
  const char* RECIPIENTS             = "Recipients";
  const char* SERVICE_CODE           = "ServiceCode";
  const char* LINKED_ID              = "LinkedID";
  const char* MESSAGE_ID             = "MessageID";
  const char* CANCEL_ID              = "CancelID";
  const char* REPLACE_ID             = "ReplaceID";
  const char* MESSAGE_CLASS          = "MessageClass";
  const char* EARLIEST_DELIVERY_TIME = "EarlestDeliveryTime";
  const char* EXPIRY_DATE            = "ExpiryDate";
  const char* DELIVERY_REPORT_TAG    = "DeliveryReport";
  const char* READ_REPLY_TAG         = "ReadReply";
  const char* PRIORITY               = "Priority";
  const char* SUBJECT                = "Subject";
  const char* CHARGED_PARTY          = "ChargedParty";
  const char* CHARGED_PARTY_ID       = "ChargedPartyID";
  const char* REPLAY_CHARGING_ID     = "ReplayChargingID";
  const char* DISTRIBUTION_INDICATOR = "DistributionIndicator";
  const char* DELIVERY_CONDITION     = "DeliveryCondition";
  const char* APPLIC_ID              = "ApplicID";
  const char* REPLY_APPLIC_ID        = "ReplyApplicID";
  const char* AUX_APPLIC_INFO        = "AuxApplicInfo";
  const char* CONTENT_CLASS          = "ContentClass";
  const char* DRM_CONTENT            = "DRMContent";
  const char* HREF                   = "href";
  const char* ALLOW_ADAPTATIONS      = "allowAdaptations";
  const char* PREVIOUSLY_SENT_BY     = "Previouslysentby";
  const char* PREVIOUSLY_SENT_DATE   = "Previouslysentdateandtime";
  const char* SENDER_SPI             = "SenderSPI";
  const char* RECIPIENT_SPI          = "RecipientSPI";
  const char* MMS_RS_ID              = "MMSRelayServerID";
  const char* MM_STATUS              = "MMStatus";
  const char* MM_STATUS_EXTENSION    = "MMStatusExtension";
  const char* STATUS_TEXT            = "StatusText";
  const char* DATE                   = "Date";
  const char* STATUS                 = "Status";
  const char* STATUS_CODE            = "StatusCode";
  const char* FAULT                  = "Fault";
  const char* FAULT_CODE             = "faultcode";
  const char* FAULT_STRING           = "faultstring";
  const char* DETAIL                 = "detail";
  
  const char* MUST_UNDERSTAND        = "env:mustUnderstand";
  const char* ENCODING_STYLE         = "env:encodingStyle";
  const char* ACTOR                  = "env:actor";
  const char* ENV_PREFIX             = "env";
  const char* MM7_PREFIX             = "mm7";
  const char* XMLNS                  = "xmlns";
  const char* XMLNS_MM7              = "xmlns:mm7";

  const char* ENVELOPE               = "Envelope";
  const char* HEADER                 = "Header";
  const char* BODY                   = "Body";
  const char* ENVELOPE_URI           = "http://schemas.xmlsoap.org/soap/envelope/";
  const char* MM7_URI                = "http://www.3gpp.org/ftp/Specs/archive/23_series/23.140/schema/REL-6-MM7-1-4";
  const char* SCHEMA_LOCATION        = "http://www.3gpp.org/ftp/Specs/archive/23_series/23.140/schema/REL-6-MM7-1-4 ./REL-6-MM7-1-4.xsd";
  
  const char* SENDER_ID             = "SenderIdentification";
  const char* MM7_VERSION           = "MM7Version";
  const char* VAS_ID                = "VASID";
  const char* VASP_ID               = "VASPID";
  const char* MMS_RELAY_SERVER_ID   = "MMSRelayServerID";
  
  const char* UTF_16                = "UTF-16";
  const char* UTF_8                 = "UTF-8";
}

}//mms
}//transport
}//scag
