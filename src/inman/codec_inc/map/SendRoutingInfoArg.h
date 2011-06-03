#ifndef	_SendRoutingInfoArg_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SendRoutingInfoArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ISDN-AddressString.h>
#include <NumberOfForwarding.h>
#include <InterrogationType.h>
#include <NULL.h>
#include <OR-Phase.h>
#include <CallReferenceNumber.h>
#include <ForwardingReason.h>
#include <SuppressionOfAnnouncement.h>
#include <AlertingPattern.h>
#include <SupportedCCBS-Phase.h>
#include <IST-SupportIndicator.h>
#include <CallDiversionTreatmentIndicator.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CUG_CheckInfo;
struct Ext_BasicServiceCode;
struct ExternalSignalInfo;
struct CamelInfo;
struct ExtensionContainer;
struct Ext_ExternalSignalInfo;

/* SendRoutingInfoArg */
typedef struct SendRoutingInfoArg {
	ISDN_AddressString_t	 msisdn;
	struct CUG_CheckInfo	*cug_CheckInfo	/* OPTIONAL */;
	NumberOfForwarding_t	*numberOfForwarding	/* OPTIONAL */;
	InterrogationType_t	 interrogationType;
	NULL_t	*or_Interrogation	/* OPTIONAL */;
	OR_Phase_t	*or_Capability	/* OPTIONAL */;
	ISDN_AddressString_t	 gmsc_OrGsmSCF_Address;
	CallReferenceNumber_t	*callReferenceNumber	/* OPTIONAL */;
	ForwardingReason_t	*forwardingReason	/* OPTIONAL */;
	struct Ext_BasicServiceCode	*basicServiceGroup	/* OPTIONAL */;
	struct ExternalSignalInfo	*networkSignalInfo	/* OPTIONAL */;
	struct CamelInfo	*camelInfo	/* OPTIONAL */;
	SuppressionOfAnnouncement_t	*suppressionOfAnnouncement	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	AlertingPattern_t	*alertingPattern	/* OPTIONAL */;
	NULL_t	*ccbs_Call	/* OPTIONAL */;
	SupportedCCBS_Phase_t	*supportedCCBS_Phase	/* OPTIONAL */;
	struct Ext_ExternalSignalInfo	*additionalSignalInfo	/* OPTIONAL */;
	IST_SupportIndicator_t	*istSupportIndicator	/* OPTIONAL */;
	NULL_t	*pre_pagingSupported	/* OPTIONAL */;
	CallDiversionTreatmentIndicator_t	*callDiversionTreatmentIndicator	/* OPTIONAL */;
	NULL_t	*longFTN_Supported	/* OPTIONAL */;
	NULL_t	*suppress_VT_CSI	/* OPTIONAL */;
	NULL_t	*suppressIncomingCallBarring	/* OPTIONAL */;
	NULL_t	*gsmSCF_InitiatedCall	/* OPTIONAL */;
	struct Ext_BasicServiceCode	*basicServiceGroup2	/* OPTIONAL */;
	struct ExternalSignalInfo	*networkSignalInfo2	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SendRoutingInfoArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoArg;

/* Referred external types */
#include <CUG-CheckInfo.h>
#include <Ext-BasicServiceCode.h>
#include <ExternalSignalInfo.h>
#include <CamelInfo.h>
#include <ExtensionContainer.h>
#include <Ext-ExternalSignalInfo.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SendRoutingInfoArg_H_ */
