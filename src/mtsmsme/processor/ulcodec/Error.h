#ifndef	_Error_H_
#define	_Error_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <OBJECT_IDENTIFIER.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Error_PR {
	Error_PR_NOTHING,	/* No components present */
	Error_PR_local,
	Error_PR_global
} Error_PR;

/* Error */
typedef struct Error {
	Error_PR present;
	union Error_u {
		long	 local;
		OBJECT_IDENTIFIER_t	 global;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Error_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Error;

#ifdef __cplusplus
}
#endif

#endif	/* _Error_H_ */
