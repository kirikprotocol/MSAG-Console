#ifndef	_PDP_ContextInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_PDP_ContextInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ContextId.h>
#include <NULL.h>
#include <PDP-Type.h>
#include <PDP-Address.h>
#include <APN.h>
#include <NSAPI.h>
#include <TransactionId.h>
#include <TEID.h>
#include <GSN-Address.h>
#include <Ext-QoS-Subscribed.h>
#include <GPRSChargingID.h>
#include <ChargingCharacteristics.h>
#include <Ext2-QoS-Subscribed.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* PDP-ContextInfo */
typedef struct PDP_ContextInfo {
	ContextId_t	 pdp_ContextIdentifier;
	NULL_t	*pdp_ContextActive	/* OPTIONAL */;
	PDP_Type_t	 pdp_Type;
	PDP_Address_t	*pdp_Address	/* OPTIONAL */;
	APN_t	*apn_Subscribed	/* OPTIONAL */;
	APN_t	*apn_InUse	/* OPTIONAL */;
	NSAPI_t	*nsapi	/* OPTIONAL */;
	TransactionId_t	*transactionId	/* OPTIONAL */;
	TEID_t	*teid_ForGnAndGp	/* OPTIONAL */;
	TEID_t	*teid_ForIu	/* OPTIONAL */;
	GSN_Address_t	*ggsn_Address	/* OPTIONAL */;
	Ext_QoS_Subscribed_t	*qos_Subscribed	/* OPTIONAL */;
	Ext_QoS_Subscribed_t	*qos_Requested	/* OPTIONAL */;
	Ext_QoS_Subscribed_t	*qos_Negotiated	/* OPTIONAL */;
	GPRSChargingID_t	*chargingId	/* OPTIONAL */;
	ChargingCharacteristics_t	*chargingCharacteristics	/* OPTIONAL */;
	GSN_Address_t	*rnc_Address	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	Ext2_QoS_Subscribed_t	*qos2_Subscribed	/* OPTIONAL */;
	Ext2_QoS_Subscribed_t	*qos2_Requested	/* OPTIONAL */;
	Ext2_QoS_Subscribed_t	*qos2_Negotiated	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PDP_ContextInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PDP_ContextInfo;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _PDP_ContextInfo_H_ */
