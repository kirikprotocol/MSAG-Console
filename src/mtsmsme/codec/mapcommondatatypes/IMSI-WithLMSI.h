#ifndef	_IMSI_WithLMSI_H_
#define	_IMSI_WithLMSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include "LMSI.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IMSI-WithLMSI */
typedef struct IMSI_WithLMSI {
	IMSI_t	 imsi;
	LMSI_t	 lmsi;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IMSI_WithLMSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IMSI_WithLMSI;

#ifdef __cplusplus
}
#endif

#endif	/* _IMSI_WithLMSI_H_ */
