#ifndef	_SS_Code_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_SS_Code_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SS-Code */
typedef OCTET_STRING_t	 SS_Code_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SS_Code;
asn_struct_free_f SS_Code_free;
asn_struct_print_f SS_Code_print;
asn_constr_check_f SS_Code_constraint;
ber_type_decoder_f SS_Code_decode_ber;
der_type_encoder_f SS_Code_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f SS_Code_decode_xer;
xer_type_encoder_f SS_Code_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _SS_Code_H_ */
