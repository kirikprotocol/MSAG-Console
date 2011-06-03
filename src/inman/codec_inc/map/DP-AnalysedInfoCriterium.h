#ifndef	_DP_AnalysedInfoCriterium_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_DP_AnalysedInfoCriterium_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ISDN-AddressString.h>
#include <ServiceKey.h>
#include <DefaultCallHandling.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* DP-AnalysedInfoCriterium */
typedef struct DP_AnalysedInfoCriterium {
	ISDN_AddressString_t	 dialledNumber;
	ServiceKey_t	 serviceKey;
	ISDN_AddressString_t	 gsmSCF_Address;
	DefaultCallHandling_t	 defaultCallHandling;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DP_AnalysedInfoCriterium_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DP_AnalysedInfoCriterium;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _DP_AnalysedInfoCriterium_H_ */
