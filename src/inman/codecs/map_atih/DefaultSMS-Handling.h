#ident "$Id$"

#ifndef	_DefaultSMS_Handling_H_
#define	_DefaultSMS_Handling_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum DefaultSMS_Handling {
	DefaultSMS_Handling_continueTransaction	= 0,
	DefaultSMS_Handling_releaseTransaction	= 1
	/*
	 * Enumeration is extensible
	 */
} DefaultSMS_Handling_e;

/* DefaultSMS-Handling */
typedef long	 DefaultSMS_Handling_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DefaultSMS_Handling;
asn_struct_free_f DefaultSMS_Handling_free;
asn_struct_print_f DefaultSMS_Handling_print;
asn_constr_check_f DefaultSMS_Handling_constraint;
ber_type_decoder_f DefaultSMS_Handling_decode_ber;
der_type_encoder_f DefaultSMS_Handling_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f DefaultSMS_Handling_decode_xer;
xer_type_encoder_f DefaultSMS_Handling_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _DefaultSMS_Handling_H_ */
