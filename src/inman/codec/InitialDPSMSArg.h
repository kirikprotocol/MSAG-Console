#ifndef	_InitialDPSMSArg_H_
#define	_InitialDPSMSArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ServiceKey.h>
#include <CalledPartyBCDNumber.h>
#include <SMS-AddressString.h>
#include <EventTypeSMS.h>
#include <IMSI.h>
#include <ISDN-AddressString.h>
#include <TimeAndTimezone.h>
#include <TPShortMessageSpecificInfo.h>
#include <TPProtocolIdentifier.h>
#include <TPDataCodingScheme.h>
#include <TPValidityPeriod.h>
#include <CallReferenceNumber.h>
#include <MS-Classmark2.h>
#include <IMEI.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct LocationInformation;
struct LocationInformationGPRS;
struct Extensions;
struct GPRSMSClass;

/* InitialDPSMSArg */
typedef struct InitialDPSMSArg {
	ServiceKey_t	 serviceKey;
	CalledPartyBCDNumber_t	*destinationSubscriberNumber	/* OPTIONAL */;
	SMS_AddressString_t	*callingPartyNumber	/* OPTIONAL */;
	EventTypeSMS_t	*eventTypeSMS	/* OPTIONAL */;
	IMSI_t	*iMSI	/* OPTIONAL */;
	struct LocationInformation	*locationInformationMSC	/* OPTIONAL */;
	struct LocationInformationGPRS	*locationInformationGPRS	/* OPTIONAL */;
	ISDN_AddressString_t	*sMSCAddress	/* OPTIONAL */;
	TimeAndTimezone_t	*timeAndTimezone	/* OPTIONAL */;
	TPShortMessageSpecificInfo_t	*tPShortMessageSpecificInfo	/* OPTIONAL */;
	TPProtocolIdentifier_t	*tPProtocolIdentifier	/* OPTIONAL */;
	TPDataCodingScheme_t	*tPDataCodingScheme	/* OPTIONAL */;
	TPValidityPeriod_t	*tPValidityPeriod	/* OPTIONAL */;
	struct Extensions	*extensions	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	CallReferenceNumber_t	*smsReferenceNumber	/* OPTIONAL */;
	ISDN_AddressString_t	*mscAddress	/* OPTIONAL */;
	ISDN_AddressString_t	*sgsn_Number	/* OPTIONAL */;
	MS_Classmark2_t	*ms_Classmark2	/* OPTIONAL */;
	struct GPRSMSClass	*gPRSMSClass	/* OPTIONAL */;
	IMEI_t	*iMEI	/* OPTIONAL */;
	ISDN_AddressString_t	*calledPartyNumber	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} InitialDPSMSArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InitialDPSMSArg;

/* Referred external types */
#include <LocationInformation.h>
#include <LocationInformationGPRS.h>
#include <Extensions.h>
#include <GPRSMSClass.h>

#ifdef __cplusplus
}
#endif

#endif	/* _InitialDPSMSArg_H_ */
