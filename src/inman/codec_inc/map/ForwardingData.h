#ifndef	_ForwardingData_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ForwardingData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ISDN-AddressString.h>
#include <ISDN-SubaddressString.h>
#include <ForwardingOptions.h>
#include <FTN-AddressString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* ForwardingData */
typedef struct ForwardingData {
	ISDN_AddressString_t	*forwardedToNumber	/* OPTIONAL */;
	ISDN_SubaddressString_t	*forwardedToSubaddress	/* OPTIONAL */;
	ForwardingOptions_t	*forwardingOptions	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	FTN_AddressString_t	*longForwardedToNumber	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ForwardingData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ForwardingData;

/* Referred external types */
#include <ExtensionContainer.h>

#ifdef __cplusplus
}
#endif

#endif	/* _ForwardingData_H_ */
