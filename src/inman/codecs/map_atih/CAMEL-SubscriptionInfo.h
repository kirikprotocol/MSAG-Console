#ident "$Id$"

#ifndef	_CAMEL_SubscriptionInfo_H_
#define	_CAMEL_SubscriptionInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include <SpecificCSI-Withdraw.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct O_CSI;
struct O_BcsmCamelTDPCriteriaList;
struct D_CSI;
struct T_CSI;
struct T_BCSM_CAMEL_TDP_CriteriaList;
struct GPRS_CSI;
struct SMS_CSI;
struct SS_CSI;
struct M_CSI;
struct ExtensionContainer;
struct MT_smsCAMELTDP_CriteriaList;
struct MG_CSI;

/* CAMEL-SubscriptionInfo */
typedef struct CAMEL_SubscriptionInfo {
	struct O_CSI	*o_CSI	/* OPTIONAL */;
	struct O_BcsmCamelTDPCriteriaList	*o_BcsmCamelTDP_CriteriaList	/* OPTIONAL */;
	struct D_CSI	*d_CSI	/* OPTIONAL */;
	struct T_CSI	*t_CSI	/* OPTIONAL */;
	struct T_BCSM_CAMEL_TDP_CriteriaList	*t_BCSM_CAMEL_TDP_CriteriaList	/* OPTIONAL */;
	struct T_CSI	*vt_CSI	/* OPTIONAL */;
	struct T_BCSM_CAMEL_TDP_CriteriaList	*vt_BCSM_CAMEL_TDP_CriteriaList	/* OPTIONAL */;
	NULL_t	*tif_CSI	/* OPTIONAL */;
	NULL_t	*tif_CSI_NotificationToCSE	/* OPTIONAL */;
	struct GPRS_CSI	*gprs_CSI	/* OPTIONAL */;
	struct SMS_CSI	*mo_sms_CSI	/* OPTIONAL */;
	struct SS_CSI	*ss_CSI	/* OPTIONAL */;
	struct M_CSI	*m_CSI	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	SpecificCSI_Withdraw_t	*specificCSIDeletedList	/* OPTIONAL */;
	struct SMS_CSI	*mt_sms_CSI	/* OPTIONAL */;
	struct MT_smsCAMELTDP_CriteriaList	*mt_smsCAMELTDP_CriteriaList	/* OPTIONAL */;
	struct MG_CSI	*mg_csi	/* OPTIONAL */;
	struct O_CSI	*o_IM_CSI	/* OPTIONAL */;
	struct O_BcsmCamelTDPCriteriaList	*o_IM_BcsmCamelTDP_CriteriaList	/* OPTIONAL */;
	struct D_CSI	*d_IM_CSI	/* OPTIONAL */;
	struct T_CSI	*vt_IM_CSI	/* OPTIONAL */;
	struct T_BCSM_CAMEL_TDP_CriteriaList	*vt_IM_BCSM_CAMEL_TDP_CriteriaList	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CAMEL_SubscriptionInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CAMEL_SubscriptionInfo;

/* Referred external types */
#include <O-CSI.h>
#include <O-BcsmCamelTDPCriteriaList.h>
#include <D-CSI.h>
#include <T-CSI.h>
#include <T-BCSM-CAMEL-TDP-CriteriaList.h>
#include <GPRS-CSI.h>
#include <SMS-CSI.h>
#include <SS-CSI.h>
#include <M-CSI.h>
#include <ExtensionContainer.h>
#include <MT-smsCAMELTDP-CriteriaList.h>
#include <MG-CSI.h>

#ifdef __cplusplus
}
#endif

#endif	/* _CAMEL_SubscriptionInfo_H_ */
