#ident "$Id$"

#ifndef	_BitStrValue_H_
#define	_BitStrValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BitStrValue */
typedef BIT_STRING_t	 BitStrValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BitStrValue;
asn_struct_free_f BitStrValue_free;
asn_struct_print_f BitStrValue_print;
asn_constr_check_f BitStrValue_constraint;
ber_type_decoder_f BitStrValue_decode_ber;
der_type_encoder_f BitStrValue_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f BitStrValue_decode_xer;
xer_type_encoder_f BitStrValue_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _BitStrValue_H_ */
