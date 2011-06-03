#ifndef	_GmscCamelSubscriptionInfo_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_GmscCamelSubscriptionInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct T_CSI;
struct O_CSI;
struct ExtensionContainer;
struct O_BcsmCamelTDPCriteriaList;
struct T_BCSM_CAMEL_TDP_CriteriaList;
struct D_CSI;

/* GmscCamelSubscriptionInfo */
typedef struct GmscCamelSubscriptionInfo {
	struct T_CSI	*t_CSI	/* OPTIONAL */;
	struct O_CSI	*o_CSI	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	struct O_BcsmCamelTDPCriteriaList	*o_BcsmCamelTDP_CriteriaList	/* OPTIONAL */;
	struct T_BCSM_CAMEL_TDP_CriteriaList	*t_BCSM_CAMEL_TDP_CriteriaList	/* OPTIONAL */;
	struct D_CSI	*d_csi	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GmscCamelSubscriptionInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GmscCamelSubscriptionInfo;

/* Referred external types */
#include <T-CSI.h>
#include <O-CSI.h>
#include <ExtensionContainer.h>
#include <O-BcsmCamelTDPCriteriaList.h>
#include <T-BCSM-CAMEL-TDP-CriteriaList.h>
#include <D-CSI.h>

#ifdef __cplusplus
}
#endif

#endif	/* _GmscCamelSubscriptionInfo_H_ */
