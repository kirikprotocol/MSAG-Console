/*
	$Id$
*/

#include "util/debug.h"
#include <inttypes.h>
#include <algorithm>

#define SMPP_USE_BUFFER
#define SMPP_SHARE_BUFFER_MEMORY

#define smartMalloc(x) malloc(x)
#define smartFree(x)   free(x)

#if defined SMPP_USE_BUFFER
#define __check_stream_invariant__ ( stream) \
	__require__ ( stream->buffer != NULL ); \
	__require__ ( stream->dataLength >= 0 ); \
	__require__ ( stream->dataOffset >= 0 ); \
	__require__ ( stream->bufferSize >= 0 ); \
	__require__ ( stream->bufferSize >= stream->dataLength ); \
	__require__ ( stream->dataOffset <= stream->dataLength );
#else
#define __check_stream_invariant__ ( stream) \
	__require__ ( stream->chanel != NULL ); \
	__require__ ( stream->dataLength >= 0 ); \
	__require__ ( stream->dataOffset >= 0 ); \
	__require__ ( stream->dataOffset <= stream->dataLength );
#endif

namespace smsc {
namespace smpp {

struct SmppStream
{
	SmppHeader header;
	unsigned int dataLength;
	unsigned int dataOffset;
#if defined SMPP_USE_BUFFER
	unsigned int bufferSize;
	unsigned char* buffer;
#else
	??? chanel;
#endif
};

template < class T >
inline T& fetchX(SmppStream* stream,T& data)
{
#if defined SMPP_USE_BUFFER	
	__check_smpp_stream_invariant__ ( stream );
	__require__ ( stream->dataOffset+sizeof(T) <= stream->dataLength );
	data = *((T*)stream->buffer);
	stream->dataOffset+sizeof(T);
	return data;
#else
#error "undefined rules of fetchX"
#endif
}

/**
	C Octet String
	двлее везде предпологается и проверяется что строка читается ОДИН раз!
*/
struct COStr
{
	// pointer on text
	char* text;
	COStr() : text(0) {};
	~COStr()
	{
#if defined (SMPP_SHARE_BUFFER_MEMORY)
	// nothing
#else
	if ( text )
		smartFree(text);
#endif
	}
	operator const char*(){__require__(text!=NULL); return text;}
	const char* cstr(){__require__(text!=NULL); return text;}
	operator void* new(size_t size) { return smartMalloc(size); }
	operator void delete(void* mem) { smartFree(mem); }
};

/**
	Octet String
	двлее везде предпологается и проверяется что строка читается ОДИН раз!
*/
struct OStr
{
	// pointer on text
	char* text;
	uint16_t length;
	OStr() : text(0),length(0) {};
	~OStr()
	{
#if defined (SMPP_SHARE_BUFFER_MEMORY)
	// nothing
#else
	if ( text )
		smartFree(text);
#endif
	}
	operator const char*(){__require__ (text!=NULL); return text;}
	const char* cstr(){__require__ (text!=NULL); return text;}
	uint16_t size() { __require__ (text!=NULL);return length; }
	operator void* new(size_t size) { return smartMalloc(size); }
	operator void delete(void* mem) { smartFree(mem); }
};

namespace SmppComandSet{ /* SMPP v3.4 (5.1.2.1) */
	static const int32_t GenericNack                 = 0x80000000;
	static const int32_t BindReceiver                = 0x00000001;
	static const int32_t BindReceiverResp            = 0x80000001;
	static const int32_t BindTransmitter             = 0x00000002;
	static const int32_t BindTransmitterResp         = 0x80000002;
	static const int32_t QuerySm                     = 0x00000003;
	static const int32_t QuerySmResp                 = 0x80000003;
	static const int32_t SubmitSm                    = 0x00000004;
	static const int32_t SubmitSmResp                = 0x80000004;
	static const int32_t DeliverSm                   = 0x00000005;
	static const int32_t DeliverSmResp               = 0x80000005;
	static const int32_t Unbind                      = 0x00000006;
	static const int32_t UnbindResp                  = 0x80000006;
	static const int32_t ReplaceSm                   = 0x00000007;
	static const int32_t ReplaceSmResp               = 0x80000007;
	static const int32_t CancelSm                    = 0x00000008;
	static const int32_t CancelSmResp                = 0x80000008;
	static const int32_t BindTransceiver             = 0x00000009;
	static const int32_t BindTransceiverResp         = 0x80000009;
	static const int32_t Outbind                     = 0x0000000b;
	static const int32_t EnquireLink                 = 0x00000015;
	static const int32_t EnquireLinkResp             = 0x80000015;
	static const int32_t SubmitMulti                 = 0x00000021;
	static const int32_t SubmitMultiResp             = 0x80000021;
	static const int32_t AlertNotification           = 0x00000102;
	static const int32_t DataSm                      = 0x00000103;
	static const int32_t DataSmResp                  = 0x80000103;
};

namespace AddrSubunitValue{	/* SMPP v3.4 (5.3.2.1) */
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
	static const uint8_t IS95_CDMA				 = 0x3;
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

struct SmppHeader
{
	uint32_t commandLength;
	int32_t commandId;
	int32_t commandStatus;
	uint32_t sequenceNumber;
};

#if defined SMPP_USE_BUFFER
inline void assignStreamWith(SmppStream* stream,void* buffer,int bufferSize)
#else
inline void assignStreamWith(SmppStream* stream,??? chanel)
#endif
{
	__require__ ( buffer != NULL );
	__require__ ( stream != NULL );
	__require__ ( bufferSize >= 16 );
#if defined SMPP_USE_BUFFER
	stream->buffer = buffer;
#else
	chanel = ???
#endif	
	stream->dataOffset = 0;
	stream->bufferSize = bufferSize;
	stream->dataLength = 4;
	fetchX(stream,stream->header.commandLength);
	stream->dataLength = header.commandLength;
	fetchX(stream,stream->header.commandId);
	fetchX(stream,stream->header.commandStatus);
	fetchX(stream,stream->header.sequenceNumber);
}

inline void fetchSmppHeader(SmppStream* stream,SmppHeader& header)
{
	__check_smpp_stream_invariant__ ( stream );
	header = stream->header;
}

inline int32_t smppCommandId(SmppStream* stream)
{
	__check_smpp_stream_invariant__ ( stream );
	return stream->header.commandId;
}

/**
	Вытаскиваем из потока строку
	@param stream  поток
	@param costr   строка
	В случае возникновения ошибки бросает исключение
*/
inline void fetchCOctetStr(SmppStream* stream,COStr& costr,int cOctMax)
{
	int length = 0;
	int maxLength;
	__check_smpp_stream_invariant__ ( stream );
	
	maxLength = std::min ( stream->dataLength-stream->dataOffset, cOctMax );

#if defined SMPP_SHARE_BUFFER_MEMORY
{
	char* tmp = stream->buffer+stream->dataOffset;
	for ( ; (length < maxLength) && (*tmp != 0) ; ++length,++tmp );
	if (*tmp) throw BadStreamException();
	costr.text = stream->buffer;
}
#else // use memory manager & stream access
{
	__require__( costr.text == NULL );
	costr.text = smartMalloc(maxLength);
	for ( ;length < maxLength; ++length)
	{
		if ( fetchX(stream,costr.text[length]) == 0 ) goto success;
	}
	throw BadStreamException();
success: ;
}
#endif
	
	stream->dataOffset += length;
	__check_smpp_stream_invariant__ ( stream );
}

/**
	Вытаскиваем из потока строку
	@param stream  поток
	@param ostr   строка
	В случае возникновения ошибки бросает исключение
*/
inline void fetchOctetStr(SmppStream* stream,OStr& ostr,uint16_t octets)
{
	__check_smpp_stream_invariant__ ( stream );
	__require__ ( octets <= stream->dataLength-stream->dataOffset );

#if defined SMPP_SHARE_BUFFER_MEMORY
{
	ostr.text = stream->buffer;
}
#else // use memory manager & stream access
{
	uint16_t length = 0;
	__require__( ostr.text == NULL );
	ostr.text = smartMalloc(octets);
	for ( ;length < octets; ++length)
	{
		fetchX(stream,ostr.text[length]);
	}
}
#endif
	stream->dataOffset += octets;
	ostr.length = octets;
	__check_smpp_stream_invariant__ ( stream );
}

/**
	Достаем опциональные поля
*/
inline void fetchSmppOptional(SmppStream* stream,SmppOptional* opt)
{
	__check_smpp_stream_invariant__ ( stream );
	while ( stream->dataOffset<stream->dataLength )
	{
		uint16_t tag;
		uint16_t length;
		int nextDataOffset;
		fetchX(stream,tag);
		fetchX(stream,length);
		
		//__require__ ( length >= 0 ); // not need because length is unsigned
		nextDataOffset = stream->dataOffset+length;

#define SMPP_HAS_FIELD(x,y)
		
		switch ( tag )
		{
#define macroFetchField(field) \
	fetchX(stream,opt->##field);\
	SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_##field);
			
#define macroFetchCOctetStr(field,maxlen) \
	fetchCOctetString(stream,opt->##field,maxlen);\
	SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_##field);
		
#define macroFetchOctetStr(field,len) \
	fetchOctetStringStr(stream,opt->##field,len);\
	SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_##field);
		
		case /*dest_addr_subunit(5.3.2.1)*/            0x05:
			macroFetchField(destAddrSubunit);
			break;
		case /*dest_network_type(5.3.2.3)*/            0x06:
			macroFetchField(destNetworkType);
			break;
		case /*dest_bearer_type(5.3.2.5)*/             0x07:
			macroFetchField(dsetBearerType);
			break;
		case /*dest_telematics_id(5.3.2.7)*/           0x08:
			macroFetchField(destTelematicsId);
			break;
		case /*source_addr_subunit(5.3.2.2)*/          0x0d:
			macroFetchField(sourceAddrSubunit);
			break;
		case /*source_network_type(5.3.2.4)*/          0x0e:
			macroFetchField(sourceNetworkType);
			break;
		case /*source_bearer_type(5.3.2.6)*/           0x0f:
			macroFetchField(sourceBearerType);
			break;
		case /*source_telematics_id(5.3.2.8)*/         0x010:
			macroFetchField(sourceTelematicsId);
			break;
		case /*qos_time_to_live(5.3.2.9)*/             0x017:
			macroFetchField(qosTimeToLive);
			break;
		case /*payload_type(5.3.2.10)*/                0x019:
			macroFetchField(pyloadType);
			break;
		case /*additional_status_info_text(5.3.2.11)*/ 0x01d:
			macroFetchCOctetStr(additionalStatisInfoText,256);
			break;
		case /*receipted_message_id(5.3.2.12)*/        0x01:
			macroFetchCOctetStr(receiptedMessageId,65);
			break;
		case /*ms_msg_wait_facilities(5.3.2.13)*/      0x030:
			macroFetchField(msMsgWaitFacilities);
			break;
		case /*privacy_indicator(5.3.2.14)*/           0x0201:
			macroFetchField(privacyIndicator);
			break;
		case /*source_subaddress(5.3.2.15)*/           0x0202:
			__require__ ( length <= 23 );
			macroFetchOctetStr(sourceSubaddress,length);
			break;
		case /*dest_subaddress(5.3.2.16)*/             0x0203:
			__require__ ( length <= 23 );
			macroFetchOctetStr(destSubaddress,length);
			break;
		case /*user_message_reference(5.3.2.17)*/      0x0204:
			macroFetchField(userMessageReference);
			break;
		case /*user_response_code(5.3.2.18)*/          0x0205:
			macroFetchField(userResponseCode);
			break;
		case /*source_port(5.3.2.20)*/                 0x020a:
			macroFetchField(sourcePort);
			break;
		case /*destination_port(5.3.2.21)*/            0x020b:
			macroFetchField(destinationPort);
			break;
		case /*sar_msg_ref_num(5.3.2.22)*/             0x020c:
			macroFetchField(sarMsgRefNum);
			break;
		case /*language_indecator(5.3.2.19)*/          0x020d:
			macroFetchField(languageIndicator);
			break;
		case /*sar_total_segments(5.3.2.23)*/          0x020e:
			macroFetchField(sarTotalSegments);
			break;
		case /*sar_segment_seqnum(5.3.2.24)*/          0x020f:
			macroFetchField(sarSegmentSegnum);
			break;
		case /*sc_interface_version(5.3.2.25)*/        0x0210:
			macroFetchField(scInterfaceVersion);
			break;
		case /*callback_num_pres_ind(5.3.2.37)*/       0x0302:
			macroFetchField(callback_num_press_ind);
			break;
		case /*callback_num_atag(5.3.2.38)*/           0x0303:
			__require__ ( length <= 65 );
			macroFetchOctetStr(callbackNumAtag,length);
			break;
		case /*number_of_messages(5.3.2.39)*/          0x0304:
			macroFetchField(numberOfMessages);
			break;
		case /*callback_num(5.3.2.36)*/                0x0381:
			__require__ ( length <= 19 );
			macroFetchOctetStr(callbackNum,length);
			break;
		case /*dpf_result(5.3.2.28)*/                  0x0420:
			macroFetchField(dpfRequit);
			break;
		case /*set_dpf(5.3.2.29)*/                     0x0421:
			macroFetchField(setDpf);
			break;
		case /*ms_available_status(5.3.2.30)*/         0x0422:
			macroFetchField(msAvilableStatus);
			break;
		case /*network_error_code(5.3.2.31)*/          0x0423:
			__require__ ( length == 3 );
			fetchX(stream,opt->networkErrCode[0]);
			fetchX(stream,opt->networkErrCode[1]);
			fetchX(stream,opt->networkErrCode[2]);
			SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_networkErrCode);
			break;
		case /*message_payload(5.3.2.32)*/             0x0424:
			macroFetchOctetStr(messagePyload,length);
			break;
		case /*delivery_failure_reason(5.3.2.33)*/     0x0425:
			macroFetchField(deliveryFailureReason);
			break;
		case /*more_messages_to_send(5.3.2.34)*/       0x0426:
			macroFetchField(moreMessagesToSend);
			break;
		case /*message_state(5.3.2.35)*/               0x0427:
			macroFetchField(messageState);
			break;
		case /*ussd_service_op(5.3.2.44)*/             0x0501:
			/*  зачитываем и храним как 1 октет ,а не как строку */
			macroFetchField(ussdServiceOp);
			break;
		case /*display_time(5.3.2.26)*/                0x1201:
			macroFetchField(displayTime);
			break;
		case /*sms_signal(5.3.2.40)*/                  0x1203:
			macroFetchField(smsSignal);
			break;
		case /*ms_validity(5.3.2.27)*/                 0x1204:
			macroFetchField(msVakidity);
			break;
		case /*alert_on_message_delivery(5.3.2.41)*/   0x130c:
			alertOnMessageDelivery = true;
			SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_alertOnMessageDelivery);
			break;
		case /*its_reply_type(5.3.2.42)*/              0x1380:
			macroFetchField(itsReplayType);
			break;
		case /*its_session_info(5.3.2.43)*/            0x1383:
			fetchX(stream,opt->itsSessionInfo[0]);
			fetchX(stream,opt->itsSessionInfo[1]);
			SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_itsSessionInfo);
			break;
		}
		__require__ ( nextDataOffset >= stream->dataOffset );
		if ( nextDataOffset > stream->dataOffset )
		{
			warning("length (%d) of field with tag %x is great field type size, fixed!\n",
							(unsigned int)length,
							(unsigned int)tag);
			while( stream->dataOffset<nextDataOffset) {int8_t octet, fetchX(stream,octet);};
		}
#undef macroFetchField
#undef macroFetchOctetStr
#undef macroFetchCOctetStr
#undef SMPP_HAS_FIELD
	}
	__check_smpp_stream_invariant__ ( stream );
}


struct PduAddress
{
	uint8_t ton;
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
	PduAddress addressRage;
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


struct PduDestAddress 
{
	uint8_t flag;
	uint8_t ton;
	uint8_t numberingPlan;
	COStr value;
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
	uint8_t      no_unsuccess;
 	UnsuccessDeliveries* sme;
};

}; // smpp namespace
}; // smsc namespace
