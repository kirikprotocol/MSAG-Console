#ident "$Id$"

#ifndef	_IntValue_H_
#define	_IntValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IntValue */
typedef long	 IntValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IntValue;
asn_struct_free_f IntValue_free;
asn_struct_print_f IntValue_print;
asn_constr_check_f IntValue_constraint;
ber_type_decoder_f IntValue_decode_ber;
der_type_encoder_f IntValue_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f IntValue_decode_xer;
xer_type_encoder_f IntValue_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _IntValue_H_ */
