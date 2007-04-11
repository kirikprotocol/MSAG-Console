#ifndef	_ReturnError_H_
#define	_ReturnError_H_


#include <asn_application.h>

/* Including external dependencies */
#include "InvokeIdType.h"
#include "Error.h"
#include <ANY.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReturnError */
typedef struct ReturnError {
	InvokeIdType_t	 invokeID;
	Error_t	 errorCode;
	ANY_t	*parameter	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ReturnError_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReturnError;

#ifdef __cplusplus
}
#endif

#endif	/* _ReturnError_H_ */
