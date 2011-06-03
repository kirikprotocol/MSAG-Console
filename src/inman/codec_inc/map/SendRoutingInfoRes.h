#ifndef	_SendRoutingInfoRes_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SendRoutingInfoRes_H_


#include <asn_application.h>

/* Including external dependencies */
#include <IMSI.h>
#include <NULL.h>
#include <ISDN-AddressString.h>
#include <NumberPortabilityStatus.h>
#include <IST-AlertTimerValue.h>
#include <SupportedCamelPhases.h>
#include <OfferedCamel4CSIs.h>
#include <AllowedServices.h>
#include <UnavailabilityCause.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtendedRoutingInfo;
struct CUG_CheckInfo;
struct SubscriberInfo;
struct SS_List;
struct Ext_BasicServiceCode;
struct ExtensionContainer;
struct NAEA_PreferredCI;
struct CCBS_Indicators;
struct RoutingInfo;
struct ExternalSignalInfo;

/* SendRoutingInfoRes */
typedef struct SendRoutingInfoRes {
	IMSI_t	*imsi	/* OPTIONAL */;
	struct ExtendedRoutingInfo	*extendedRoutingInfo	/* OPTIONAL */;
	struct CUG_CheckInfo	*cug_CheckInfo	/* OPTIONAL */;
	NULL_t	*cugSubscriptionFlag	/* OPTIONAL */;
	struct SubscriberInfo	*subscriberInfo	/* OPTIONAL */;
	struct SS_List	*ss_List	/* OPTIONAL */;
	struct Ext_BasicServiceCode	*basicService	/* OPTIONAL */;
	NULL_t	*forwardingInterrogationRequired	/* OPTIONAL */;
	ISDN_AddressString_t	*vmsc_Address	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct NAEA_PreferredCI	*naea_PreferredCI	/* OPTIONAL */;
	struct CCBS_Indicators	*ccbs_Indicators	/* OPTIONAL */;
	ISDN_AddressString_t	*msisdn	/* OPTIONAL */;
	NumberPortabilityStatus_t	*numberPortabilityStatus	/* OPTIONAL */;
	IST_AlertTimerValue_t	*istAlertTimer	/* OPTIONAL */;
	SupportedCamelPhases_t	*supportedCamelPhasesInVMSC	/* OPTIONAL */;
	OfferedCamel4CSIs_t	*offeredCamel4CSIsInVMSC	/* OPTIONAL */;
	struct RoutingInfo	*routingInfo2	/* OPTIONAL */;
	struct SS_List	*ss_List2	/* OPTIONAL */;
	struct Ext_BasicServiceCode	*basicService2	/* OPTIONAL */;
	AllowedServices_t	*allowedServices	/* OPTIONAL */;
	UnavailabilityCause_t	*unavailabilityCause	/* OPTIONAL */;
	NULL_t	*releaseResourcesSupported	/* OPTIONAL */;
	struct ExternalSignalInfo	*gsm_BearerCapability	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SendRoutingInfoRes_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoRes;

/* Referred external types */
#include <ExtendedRoutingInfo.h>
#include <CUG-CheckInfo.h>
#include <SubscriberInfo.h>
#include <SS-List.h>
#include <Ext-BasicServiceCode.h>
#include <ExtensionContainer.h>
#include <NAEA-PreferredCI.h>
#include <CCBS-Indicators.h>
#include <RoutingInfo.h>
#include <ExternalSignalInfo.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SendRoutingInfoRes_H_ */
