#ifndef	_Password_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_Password_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NumericString.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Password */
typedef NumericString_t	 Password_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Password;
asn_struct_free_f Password_free;
asn_struct_print_f Password_print;
asn_constr_check_f Password_constraint;
ber_type_decoder_f Password_decode_ber;
der_type_encoder_f Password_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f Password_decode_xer;
xer_type_encoder_f Password_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _Password_H_ */
