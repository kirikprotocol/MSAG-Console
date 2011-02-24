#ident "$Id$"

#ifndef	_OIDType_H_
#define	_OIDType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OBJECT_IDENTIFIER.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OIDType */
typedef OBJECT_IDENTIFIER_t	 OIDType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_OIDType;
asn_struct_free_f OIDType_free;
asn_struct_print_f OIDType_print;
asn_constr_check_f OIDType_constraint;
ber_type_decoder_f OIDType_decode_ber;
der_type_encoder_f OIDType_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f OIDType_decode_xer;
xer_type_encoder_f OIDType_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _OIDType_H_ */
