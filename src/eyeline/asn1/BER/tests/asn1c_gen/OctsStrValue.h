#ident "$Id$"

#ifndef	_OctsStrValue_H_
#define	_OctsStrValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OctsStrValue */
typedef OCTET_STRING_t	 OctsStrValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_OctsStrValue;
asn_struct_free_f OctsStrValue_free;
asn_struct_print_f OctsStrValue_print;
asn_constr_check_f OctsStrValue_constraint;
ber_type_decoder_f OctsStrValue_decode_ber;
der_type_encoder_f OctsStrValue_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f OctsStrValue_decode_xer;
xer_type_encoder_f OctsStrValue_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _OctsStrValue_H_ */