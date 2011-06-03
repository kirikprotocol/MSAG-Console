#ifndef	_BearerServiceCode_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_BearerServiceCode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BearerServiceCode */
typedef OCTET_STRING_t	 BearerServiceCode_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BearerServiceCode;
asn_struct_free_f BearerServiceCode_free;
asn_struct_print_f BearerServiceCode_print;
asn_constr_check_f BearerServiceCode_constraint;
ber_type_decoder_f BearerServiceCode_decode_ber;
der_type_encoder_f BearerServiceCode_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f BearerServiceCode_decode_xer;
xer_type_encoder_f BearerServiceCode_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _BearerServiceCode_H_ */
