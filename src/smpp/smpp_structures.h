
namespace SmppComandSet{ /* SMPP v3.4 (5.1.2.1) */
  static const int32_t GENERIC_NACK                 = 0x80000000;
  static const int32_t BIND_RECIEVER                = 0x00000001;
  static const int32_t BIND_RECIEVER_RESP           = 0x80000001;
  static const int32_t BIND_TRANSMITTER             = 0x00000002;
  static const int32_t BIND_TRANSMITTER_RESP        = 0x80000002;
  static const int32_t QUERY_SM                     = 0x00000003;
  static const int32_t QUERY_SM_RESP                = 0x80000003;
  static const int32_t SUBMIT_SM                    = 0x00000004;
  static const int32_t SUBMIT_SM_RESP               = 0x80000004;
  static const int32_t DELIVERY_SM                  = 0x00000005;
  static const int32_t DELIVERY_SM_RESP             = 0x80000005;
  static const int32_t UNBIND                       = 0x00000006;
  static const int32_t UNBIND_RESP                  = 0x80000006;
  static const int32_t REPLACE_SM                   = 0x00000007;
  static const int32_t REPLACE_SM_RESP              = 0x80000007;
  static const int32_t CANCEL_SM                    = 0x00000008;
  static const int32_t CANCEL_SM_RESP               = 0x80000008;
  static const int32_t BIND_TRANCIEVER              = 0x00000009;
  static const int32_t BIND_TRANCIEVER_RESP         = 0x80000009;
  static const int32_t OUTBIND                      = 0x0000000b;
  static const int32_t ENQUIRE_LINK                 = 0x00000015;
  static const int32_t ENQUIRE_LINK_RESP            = 0x80000015;
  static const int32_t SUBMIT_MULTI                 = 0x00000021;
  static const int32_t SUBMIT_MULTI_RESP            = 0x80000021;
  static const int32_t ALERT_NOTIFICATION           = 0x00000102;
  static const int32_t DATA_SM                      = 0x00000103;
  static const int32_t DATA_SM_RESP                 = 0x80000103;
};

namespace AddrSubunitValue{ /* SMPP v3.4 (5.3.2.1) */
  static const uint8_t UNKNOWN          = 0;
  static const uint8_t MS_DISPLAY       = 0x1;
  static const uint8_t MOBILE_EQUIPMENT = 0x2;
  static const uint8_t SMART_CARD1      = 0x3;
  static const uint8_t EXTERNAL_UNIT    = 0x4;
};

namespace NetworkTypeValue{ /* SMPP v3.4 (5.3.2.3) */
  static const uint8_t UNKNOWN          = 0;
  static const uint8_t GSM              = 0x1;
  static const uint8_t ANSI136_TDMA     = 0x2;
  static const uint8_t IS95_CDMA         = 0x3;
  static const uint8_t PDC              = 0x4;
  static const uint8_t PHS              = 0x5;
  static const uint8_t IDEN             = 0x6;
  static const uint8_t AMPS             = 0x7;
  static const uint8_t PAGING_NETWORK   = 0x8;
};

namespace BearerTypeValue{ /* SMPP v3.4 (5.3.2.5) */
  static const uint8_t UNKNOWN          = 0;
  static const uint8_t SMS              = 0x1;
  static const uint8_t CSD              = 0x2;
  static const uint8_t PACKET_DATA      = 0x3;
  static const uint8_t USSD             = 0x4;
  static const uint8_t CDPD             = 0x5;
  static const uint8_t DATA_TAC         = 0x6;
  static const uint8_t FLEX/RE_FLEX     = 0x7;
  static const uint8_t CELLCAST         = 0x8;
};

namespace PyloadTypeValue{ /* SMPP v3.4 (5.3.2.10) */
  static const uint8_t WDP  = 0;
  static const uint8_t WCMP = 1;
};

namespace MsValidityValue{ /* SMPP v3.4 (5.3.2.27) */
  static const uint8_t STORE_INDEFINITELY          = 0;
  static const uint8_t POWER_DOWN                  = 0x1;
  static const uint8_t SID_BASED_REGISTRATION_AREA = 0x2;
  static const uint8_t DISPLAY_ONLY                = 0x3;
};

namespace MsAvailabilityStatus{ /* SMPP v3.4 (5.3.2.30) */
  static const uint8_t AVAILABLE                   = 0;
  static const uint8_t DENIED                      = 0x1;
  static const uint8_t UNAVAILABLE                 = 0x2;
};

namespace UssdServiceOpValue{/* SMPP v3.4 (5.3.2.44) */
  static const PSSD_INDICATION        = 0;
  static const PSSR_INDICATION        = 1;
  static const USSR_REQUEST           = 2;
  static const USSN_REQUEST           = 3;
  static const PSSD_RESPONSE          = 16;
  static const PSSR_RESPONSE          = 17;
  static const USSR_CONFIRM           = 18;
  static const USSN_CONFIRM           = 19;
};

struct SmppHeader
{
  uint32_t commandLength;
  int32_t commandId;
  int32_t commandStatus;
  uint32_t sequenceNumber;
};

struct SmppOptional
{
  uint8_t   destAddrSubunit;
  uint8_t   sourceAddrSubunit;
  uint8_t   destNetworkType;
  uint8_t   sourceNetworkType;
  uint8_t   destBearerType;
  uint8_t   sourceBearerType;
  uint16_t  destTelematicsId;
  uint8_t   sourceTelematicsId;
  uint32_t  qosTimeToLive;
  uint8_t   payloadType;
  COStr     additionalStatusInfoText;
  COStr     receiptedMessageId;
  uint8_t   msMsgWaitFacilities;
  uint8_t   privacyIndicator;
  OStr      sourceSubaddress;
  Ostr      destSubaddress;
  uint16_t  userMessageReference;
  uint8_t   userResponseCode;
  uint8_t   languageIndicator;
  uint16_t  sourcePort;
  uint16_t  destinationPort;
  uint16_t  sarMsgRefNum;
  uint8_t   sarTotalSegments;
  uint8_t   sarSegmentSegnum;
  uint8_t   scInterfaceVersion;
  uint8_t   displayTime;
  uint8_t   msValidity;
  uint8_t   dpfResult;
  uint8_t   setDpf;
  uint8_t   msAvilabilityStatus;
  uint8_t   networkErrorCode[3];
  OStr      messagePyload;
  uint8_t   deliveryFailureReason;
  uint8_t   moreMessagesToSend;
  uint8_t   messageState;
  OStr      callbackNum;
  uint8_t   callbackNumPresInd;
  OStr      callbackNumAtag;
  uint8_t   numberOfMessages;
  uint16_t  smsSignal;
  bool      alertOnMessageDelivery;
  uint8_t   itsReplyType;
  uint8_t   itsSessionInfo[2];
  uint8_t   ussdServiceOp;
};


struct PduOutBind
{
  SmppHeader header;
  COStr systemId;
  COStr password;
};

struct PduWithOnlyHeader
{
  SmppHeader header;
}
typedef PduWithOnlyHeader PduUnbind;
typedef PduWithOnlyHeader PduUnbindResp;
typedef PduWithOnlyHeader PduGenericNack;


struct PduDestAddress : public PduAddress
{
	uint8_t flag;
  PduDestAddress() :
    flag(0),
    addrTon(0),
    addrNpi(0) {};
};

struct PduPartSm
{
  COStr serviceType;
  PduAddress source;
  PduAddress dest;
  uint8_t numberOfDests;
  PduDestAddress* dests;
  uint8_t esmClass;
  uint8_t protocolId;
  uint8_t priorityFlag;
  COStr scheduleDeliveryTime;
  COStr validityPeriod;
  uint8_t registredDelivery;
  uint8_t replaceIfPresentFlag;
  uint8_t dataCoding;
  uint8_t smDefaultMsgId;
  uint8_t smLength;
  OStr shortMessage;
  PduPartSm() :
    numberOfDest(0),
    dests(0),
    esmClass(0),
    protocolId(0),
    priorityFlag(0),
    registredDelivery(0),
    replaceIfPresentFlag(0),
    dataCoding(0),
    smDefaultMsgId(0),
    smLength(0) {}
}

struct PduXSm
{
  SmppHeader   header;
  PduPartSm    message;
  SmppOptional optional;
};

struct PduXSmResp
{
  SmppHeader   header;
  COStr        messageId;
};

typedef PduXSm  PduSubmitSm;
typedef PduXSm  PduDeliverySm;
typedef PduXSm  PduMultiSm;

typedef PduXSmResp  PduSubmitSmResp;
typedef PduXSmResp  PduDeliverySmResp;

struct UnsuccessDeliveries
{
  PduAddress addr;
  uint32_t errorStatusCode;
};

struct PduSubmitMultiResp
{
  SmppHeader   header;
  COStr        messageId;
  uint8_t      noUnsuccess;
  UnsuccessDeliveries* sme;
};


struct PduAddress
{
  uint8_t typeOfNumber;
  uint8_t numberingPlan;
  COStr value;
  PduAddr() :
    ton(0),
    numberingPlan(0) {};
};

struct PduBindTRX
{
  SmppHeader header;
  COStr systemId;
  COStr password;
  COStr systemType;
  uint8_t interfaceVersion;
  PduAddress addressRange;
};

struct PduBindTRXResp
{
  SmppHeader header;
  COStr systemId;
//optional
  int8_t scInterfaceVersion;
  PduBindTRXResp() :
    scInterfaceVersion(0) {};
};

inline bool smppPduHasSms(SmppHeader* pdu)
{
  switch(pdu->commandId)
  {
  case SUBMIT_SM:
  case DELIVERY_SM:
  case SUBMIT_MULTI:
  case DATA_SM:
    return true;
  }
  return false;
}

