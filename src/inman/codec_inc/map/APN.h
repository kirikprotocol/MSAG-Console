#ifndef	_APN_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_APN_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* APN */
typedef OCTET_STRING_t	 APN_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_APN;
asn_struct_free_f APN_free;
asn_struct_print_f APN_print;
asn_constr_check_f APN_constraint;
ber_type_decoder_f APN_decode_ber;
der_type_encoder_f APN_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f APN_decode_xer;
xer_type_encoder_f APN_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _APN_H_ */
