#ifndef	_PDP_Address_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_PDP_Address_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PDP-Address */
typedef OCTET_STRING_t	 PDP_Address_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PDP_Address;
asn_struct_free_f PDP_Address_free;
asn_struct_print_f PDP_Address_print;
asn_constr_check_f PDP_Address_constraint;
ber_type_decoder_f PDP_Address_decode_ber;
der_type_encoder_f PDP_Address_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f PDP_Address_decode_xer;
xer_type_encoder_f PDP_Address_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _PDP_Address_H_ */
