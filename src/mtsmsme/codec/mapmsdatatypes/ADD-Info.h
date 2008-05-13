#ifndef	_ADD_Info_H_
#define	_ADD_Info_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMEI.h"
#include <NULL.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ADD-Info */
typedef struct ADD_Info {
	IMEI_t	 imeisv;
	NULL_t	*skipSubscriberDataUpdate	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ADD_Info_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ADD_Info;

#ifdef __cplusplus
}
#endif

#endif	/* _ADD_Info_H_ */
