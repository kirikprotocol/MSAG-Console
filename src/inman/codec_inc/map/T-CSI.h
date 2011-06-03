#ifndef	_T_CSI_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_T_CSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <T-BcsmCamelTDPDataList.h>
#include <CamelCapabilityHandling.h>
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* T-CSI */
typedef struct T_CSI {
	T_BcsmCamelTDPDataList_t	 t_BcsmCamelTDPDataList;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	CamelCapabilityHandling_t	*camelCapabilityHandling	/* OPTIONAL */;
	NULL_t	*notificationToCSE	/* OPTIONAL */;
	NULL_t	*csi_Active	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} T_CSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_T_CSI;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _T_CSI_H_ */
