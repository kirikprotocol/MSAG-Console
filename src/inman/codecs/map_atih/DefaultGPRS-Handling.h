#ident "$Id$"

#ifndef	_DefaultGPRS_Handling_H_
#define	_DefaultGPRS_Handling_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum DefaultGPRS_Handling {
	DefaultGPRS_Handling_continueTransaction	= 0,
	DefaultGPRS_Handling_releaseTransaction	= 1
	/*
	 * Enumeration is extensible
	 */
} DefaultGPRS_Handling_e;

/* DefaultGPRS-Handling */
typedef long	 DefaultGPRS_Handling_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DefaultGPRS_Handling;
asn_struct_free_f DefaultGPRS_Handling_free;
asn_struct_print_f DefaultGPRS_Handling_print;
asn_constr_check_f DefaultGPRS_Handling_constraint;
ber_type_decoder_f DefaultGPRS_Handling_decode_ber;
der_type_encoder_f DefaultGPRS_Handling_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f DefaultGPRS_Handling_decode_xer;
xer_type_encoder_f DefaultGPRS_Handling_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _DefaultGPRS_Handling_H_ */
