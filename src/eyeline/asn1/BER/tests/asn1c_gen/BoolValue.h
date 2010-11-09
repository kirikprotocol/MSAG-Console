#ident "$Id$"

#ifndef	_BoolValue_H_
#define	_BoolValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BOOLEAN.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BoolValue */
typedef BOOLEAN_t	 BoolValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BoolValue;
asn_struct_free_f BoolValue_free;
asn_struct_print_f BoolValue_print;
asn_constr_check_f BoolValue_constraint;
ber_type_decoder_f BoolValue_decode_ber;
der_type_encoder_f BoolValue_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f BoolValue_decode_xer;
xer_type_encoder_f BoolValue_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _BoolValue_H_ */