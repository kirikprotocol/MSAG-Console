#ifndef	_Invoke_H_
#define	_Invoke_H_


#include <asn_application.h>

/* Including external dependencies */
#include "InvokeId.h"
#include "Code.h"
#include <ANY.h>
#include <NULL.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum linkedId_PR {
	linkedId_PR_NOTHING,	/* No components present */
	linkedId_PR_present,
	linkedId_PR_absent
} linkedId_PR;

/* Invoke */
typedef struct Invoke {
	InvokeId_t	 invokeId;
	struct linkedId {
		linkedId_PR present;
		union Invoke__linkedId_u {
			InvokeId_t	 present;
			NULL_t	 absent;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *linkedId;
	Code_t	 opcode;
	ANY_t	*argument	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Invoke_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Invoke;

#ifdef __cplusplus
}
#endif

#endif	/* _Invoke_H_ */
