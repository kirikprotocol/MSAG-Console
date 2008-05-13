#ifndef	_ProvideRoamingNumberArg_H_
#define	_ProvideRoamingNumberArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include "ISDN-AddressString.h"
#include "LMSI.h"
#include "SuppressionOfAnnouncement.h"
#include "CallReferenceNumber.h"
#include <NULL.h>
#include "AlertingPattern.h"
#include "SupportedCamelPhases.h"
#include "OfferedCamel4CSIs.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExternalSignalInfo;
struct ExtensionContainer;
struct Ext_ExternalSignalInfo;

/* ProvideRoamingNumberArg */
typedef struct ProvideRoamingNumberArg {
	IMSI_t	 imsi;
	ISDN_AddressString_t	 msc_Number;
	ISDN_AddressString_t	*msisdn	/* OPTIONAL */;
	LMSI_t	*lmsi	/* OPTIONAL */;
	struct ExternalSignalInfo	*gsm_BearerCapability	/* OPTIONAL */;
	struct ExternalSignalInfo	*networkSignalInfo	/* OPTIONAL */;
	SuppressionOfAnnouncement_t	*suppressionOfAnnouncement	/* OPTIONAL */;
	ISDN_AddressString_t	*gmsc_Address	/* OPTIONAL */;
	CallReferenceNumber_t	*callReferenceNumber	/* OPTIONAL */;
	NULL_t	*or_Interrogation	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	AlertingPattern_t	*alertingPattern	/* OPTIONAL */;
	NULL_t	*ccbs_Call	/* OPTIONAL */;
	SupportedCamelPhases_t	*supportedCamelPhasesInInterrogatingNode	/* OPTIONAL */;
	struct Ext_ExternalSignalInfo	*additionalSignalInfo	/* OPTIONAL */;
	NULL_t	*orNotSupportedInGMSC	/* OPTIONAL */;
	NULL_t	*pre_pagingSupported	/* OPTIONAL */;
	NULL_t	*longFTN_Supported	/* OPTIONAL */;
	NULL_t	*suppress_VT_CSI	/* OPTIONAL */;
	OfferedCamel4CSIs_t	*offeredCamel4CSIsInInterrogatingNode	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ProvideRoamingNumberArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ProvideRoamingNumberArg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExternalSignalInfo.h"
#include "ExtensionContainer.h"
#include "Ext-ExternalSignalInfo.h"

#endif	/* _ProvideRoamingNumberArg_H_ */
