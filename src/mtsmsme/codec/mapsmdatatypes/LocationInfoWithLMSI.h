#ifndef	_LocationInfoWithLMSI_H_
#define	_LocationInfoWithLMSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include "LMSI.h"
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;
struct Additional_Number;

/* LocationInfoWithLMSI */
typedef struct LocationInfoWithLMSI {
	ISDN_AddressString_t	 networkNode_Number;
	LMSI_t	*lmsi	/* OPTIONAL */;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*gprsNodeIndicator	/* OPTIONAL */;
	struct Additional_Number	*additional_Number	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LocationInfoWithLMSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LocationInfoWithLMSI;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"
#include "Additional-Number.h"

#endif	/* _LocationInfoWithLMSI_H_ */
