#ifndef	_SubscriberInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SubscriberInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <IMEI.h>
#include <MS-Classmark2.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct LocationInformation;
struct SubscriberState;
struct ExtensionContainer;
struct LocationInformationGPRS;
struct PS_SubscriberState;
struct GPRSMSClass;
struct MNPInfoRes;

/* SubscriberInfo */
typedef struct SubscriberInfo {
	struct LocationInformation	*locationInformation	/* OPTIONAL */;
	struct SubscriberState	*subscriberState	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct LocationInformationGPRS	*locationInformationGPRS	/* OPTIONAL */;
	struct PS_SubscriberState	*ps_SubscriberState	/* OPTIONAL */;
	IMEI_t	*imei	/* OPTIONAL */;
	MS_Classmark2_t	*ms_Classmark2	/* OPTIONAL */;
	struct GPRSMSClass	*gprs_MS_Class	/* OPTIONAL */;
	struct MNPInfoRes	*mnpInfoRes	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SubscriberInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SubscriberInfo;

/* Referred external types */
#include <LocationInformation.h>
#include <SubscriberState.h>
#include <ExtensionContainer.h>
#include <LocationInformationGPRS.h>
#include <PS-SubscriberState.h>
#include <GPRSMSClass.h>
#include <MNPInfoRes.h>

#ifdef __cplusplus
}
#endif

#endif	/* _SubscriberInfo_H_ */
