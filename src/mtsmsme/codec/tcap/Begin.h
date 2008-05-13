#ifndef	_Begin_H_
#define	_Begin_H_


#include <asn_application.h>

/* Including external dependencies */
#include "OrigTransactionID.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct EXT;
struct ComponentPortion;

/* Begin */
typedef struct Begin {
	OrigTransactionID_t	 otid;
	struct EXT	*dialoguePortion	/* OPTIONAL */;
	struct ComponentPortion	*components	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Begin_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Begin;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "DialoguePortion.h"
#include "ComponentPortion.h"

#endif	/* _Begin_H_ */
