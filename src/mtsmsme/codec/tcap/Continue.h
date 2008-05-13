#ifndef	_Continue_H_
#define	_Continue_H_


#include <asn_application.h>

/* Including external dependencies */
#include "OrigTransactionID.h"
#include "DestTransactionID.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct EXT;
struct ComponentPortion;

/* Continue */
typedef struct Continue {
	OrigTransactionID_t	 otid;
	DestTransactionID_t	 dtid;
	struct EXT	*dialoguePortion	/* OPTIONAL */;
	struct ComponentPortion	*components	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Continue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Continue;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "DialoguePortion.h"
#include "ComponentPortion.h"

#endif	/* _Continue_H_ */
