#ifndef __SMSC_SMS_SMS_CONST_H__
#define __SMSC_SMS_SMS_CONST_H__

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

namespace smsc{
namespace sms{

const int USSD_PSSD_IND   = 0;
const int USSD_PSSR_IND   = 1;
const int USSD_USSR_REQ   = 2;
const int USSD_USSN_REQ   = 3;
const int USSD_PSSD_RESP  = 16;
const int USSD_PSSR_RESP  = 17;
const int USSD_USSR_CONF  = 18;
const int USSD_USSN_CONF  = 19;

/* smpp+ ussd ops*/
const int USSD_USSREL_REQ     = 128;
const int USSD_USSREL_IND     = 129;
const int USSD_USSR_REQ_LAST  = 130;
const int USSD_USSN_REQ_LAST  = 131;
const int USSD_USSR_CONF_LAST = 146;

const int USSD_USSR_REQ_VLR = 150;
const int USSD_USSN_REQ_VLR = 151;
const int USSD_USSR_REQ_VLR_LAST = 152;
const int USSD_USSN_REQ_VLR_LAST = 153;

const int BILLING_NONE     = 0;
const int BILLING_NORMAL   = 1;
const int BILLING_MT       = 2;
const int BILLING_FINALREP = 3;
const int BILLING_ONSUBMIT = 4;
const int BILLING_CDR      = 5;

const int MAX_ESERVICE_TYPE_LENGTH = 5;
const int MAX_SMESYSID_TYPE_LENGTH = 15;
const int MAX_SMEPASSWD_TYPE_LENGTH = 8;
const int MAX_ROUTE_ID_TYPE_LENGTH = 32;
const int MAX_ADDRESS_VALUE_LENGTH = 20;
const int MAX_SHORT_MESSAGE_LENGTH = 255; // Deprecated !!!
const int MAX_BODY_LENGTH          = 1500;
const int MAX_FULL_ADDRESS_VALUE_LENGTH = 30;

typedef uint64_t    SMSId;
typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];
typedef char        FullAddressValue[MAX_FULL_ADDRESS_VALUE_LENGTH+1];
typedef char        SmeSystemIdType[MAX_SMESYSID_TYPE_LENGTH+1];
typedef char        EService[MAX_ESERVICE_TYPE_LENGTH+1];
typedef char        RouteId[MAX_ROUTE_ID_TYPE_LENGTH+1];

const uint8_t SMSC_BYTE_ENROUTE_STATE       = (uint8_t)0;
const uint8_t SMSC_BYTE_DELIVERED_STATE     = (uint8_t)1;
const uint8_t SMSC_BYTE_EXPIRED_STATE       = (uint8_t)2;
const uint8_t SMSC_BYTE_UNDELIVERABLE_STATE = (uint8_t)3;
const uint8_t SMSC_BYTE_DELETED_STATE       = (uint8_t)4;

const uint8_t SMSC_DEFAULT_MSG_MODE = 0;
const uint8_t SMSC_DATAGRAM_MSG_MODE        = 1;
const uint8_t SMSC_TRANSACTION_MSG_MODE     = 2;
const uint8_t SMSC_STOREANDFORWARD_MSG_MODE = 3;

/**
*
*
* @see SMS
*/
typedef enum {
  ENROUTE         = SMSC_BYTE_ENROUTE_STATE,
  DELIVERED       = SMSC_BYTE_DELIVERED_STATE,
  EXPIRED         = SMSC_BYTE_EXPIRED_STATE,
  UNDELIVERABLE   = SMSC_BYTE_UNDELIVERABLE_STATE,
  DELETED         = SMSC_BYTE_DELETED_STATE
} State;

}
}

#endif
