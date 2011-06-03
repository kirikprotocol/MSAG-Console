#ifndef	_LSAIdentity_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_LSAIdentity_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LSAIdentity */
typedef OCTET_STRING_t	 LSAIdentity_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LSAIdentity;
asn_struct_free_f LSAIdentity_free;
asn_struct_print_f LSAIdentity_print;
asn_constr_check_f LSAIdentity_constraint;
ber_type_decoder_f LSAIdentity_decode_ber;
der_type_encoder_f LSAIdentity_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f LSAIdentity_decode_xer;
xer_type_encoder_f LSAIdentity_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _LSAIdentity_H_ */
