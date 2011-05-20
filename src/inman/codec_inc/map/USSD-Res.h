#ifndef	_USSD_Res_H_
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define	_USSD_Res_H_


#include <asn_application.h>

/* Including external dependencies */
#include <USSD-DataCodingScheme.h>
#include <USSD-String.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* USSD-Res */
typedef struct USSD_Res {
	USSD_DataCodingScheme_t	 ussd_DataCodingScheme;
	USSD_String_t	 ussd_String;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} USSD_Res_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_USSD_Res;

#ifdef __cplusplus
}
#endif

#endif	/* _USSD_Res_H_ */
