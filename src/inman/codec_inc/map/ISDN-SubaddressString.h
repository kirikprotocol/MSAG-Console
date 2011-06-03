#ifndef	_ISDN_SubaddressString_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ISDN_SubaddressString_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ISDN-SubaddressString */
typedef OCTET_STRING_t	 ISDN_SubaddressString_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ISDN_SubaddressString;
asn_struct_free_f ISDN_SubaddressString_free;
asn_struct_print_f ISDN_SubaddressString_print;
asn_constr_check_f ISDN_SubaddressString_constraint;
ber_type_decoder_f ISDN_SubaddressString_decode_ber;
der_type_encoder_f ISDN_SubaddressString_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ISDN_SubaddressString_decode_xer;
xer_type_encoder_f ISDN_SubaddressString_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ISDN_SubaddressString_H_ */
