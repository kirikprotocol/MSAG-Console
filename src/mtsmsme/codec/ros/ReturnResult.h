#ifndef	_ReturnResult_H_
#define	_ReturnResult_H_


#include <asn_application.h>

/* Including external dependencies */
#include "InvokeId.h"
#include "Code.h"
#include <ANY.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReturnResult */
typedef struct ReturnResult {
	InvokeId_t	 invokeId;
	struct result {
		Code_t	 opcode;
		ANY_t	 result;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *result;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ReturnResult_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReturnResult;

#ifdef __cplusplus
}
#endif

#endif	/* _ReturnResult_H_ */
