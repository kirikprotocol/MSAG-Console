#ident "$Id$"

#ifndef	_USSD_Arg_H_
#define	_USSD_Arg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <USSD-DataCodingScheme.h>
#include <USSD-String.h>
#include <AlertingPattern.h>
#include <ISDN-AddressString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* USSD-Arg */
typedef struct USSD_Arg {
	USSD_DataCodingScheme_t	 ussd_DataCodingScheme;
	USSD_String_t	 ussd_String;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	AlertingPattern_t	*alertingPattern	/* OPTIONAL */;
	ISDN_AddressString_t	*msisdn	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} USSD_Arg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_USSD_Arg;

#ifdef __cplusplus
}
#endif

#endif	/* _USSD_Arg_H_ */
