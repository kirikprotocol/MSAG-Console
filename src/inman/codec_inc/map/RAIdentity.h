#ifndef	_RAIdentity_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_RAIdentity_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RAIdentity */
typedef OCTET_STRING_t	 RAIdentity_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RAIdentity;
asn_struct_free_f RAIdentity_free;
asn_struct_print_f RAIdentity_print;
asn_constr_check_f RAIdentity_constraint;
ber_type_decoder_f RAIdentity_decode_ber;
der_type_encoder_f RAIdentity_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f RAIdentity_decode_xer;
xer_type_encoder_f RAIdentity_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _RAIdentity_H_ */
