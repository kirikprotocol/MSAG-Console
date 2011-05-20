#ifndef	_USSD_String_H_
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define	_USSD_String_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* USSD-String */
typedef OCTET_STRING_t	 USSD_String_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_USSD_String;
asn_struct_free_f USSD_String_free;
asn_struct_print_f USSD_String_print;
asn_constr_check_f USSD_String_constraint;
ber_type_decoder_f USSD_String_decode_ber;
der_type_encoder_f USSD_String_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f USSD_String_decode_xer;
xer_type_encoder_f USSD_String_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _USSD_String_H_ */
