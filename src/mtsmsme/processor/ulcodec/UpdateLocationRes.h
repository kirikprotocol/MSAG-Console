#ifndef	_UpdateLocationRes_H_
#define	_UpdateLocationRes_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ISDN-AddressString.h"
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ExtensionContainer;

/* UpdateLocationRes */
typedef struct UpdateLocationRes {
	ISDN_AddressString_t	 hlr_Number;
	struct ExtensionContainer	*extensionContainer	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	NULL_t	*add_Capability	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UpdateLocationRes_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UpdateLocationRes;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "ExtensionContainer.h"

#endif	/* _UpdateLocationRes_H_ */
