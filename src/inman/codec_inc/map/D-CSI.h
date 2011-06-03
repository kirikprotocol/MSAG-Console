#ifndef	_D_CSI_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_D_CSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <CamelCapabilityHandling.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct DP_AnalysedInfoCriteriaList;
struct ExtensionContainer;

/* D-CSI */
typedef struct D_CSI {
	struct DP_AnalysedInfoCriteriaList	*dp_AnalysedInfoCriteriaList	/* OPTIONAL */;
	CamelCapabilityHandling_t	*camelCapabilityHandling	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	NULL_t	*notificationToCSE	/* OPTIONAL */;
	NULL_t	*csi_Active	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} D_CSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_D_CSI;

/* Referred external types */
#include <DP-AnalysedInfoCriteriaList.h>
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _D_CSI_H_ */
