#ifndef	_BasicServiceCode_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_BasicServiceCode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BearerServiceCode.h>
#include <TeleserviceCode.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum BasicServiceCode_PR {
	BasicServiceCode_PR_NOTHING,	/* No components present */
	BasicServiceCode_PR_bearerService,
	BasicServiceCode_PR_teleservice
} BasicServiceCode_PR;

/* BasicServiceCode */
typedef struct BasicServiceCode {
	BasicServiceCode_PR present;
	union BasicServiceCode_u {
		BearerServiceCode_t	 bearerService;
		TeleserviceCode_t	 teleservice;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BasicServiceCode_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BasicServiceCode;

#ifdef __cplusplus
}
#endif

#endif	/* _BasicServiceCode_H_ */
