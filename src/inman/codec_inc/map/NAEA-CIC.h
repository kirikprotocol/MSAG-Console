#ifndef	_NAEA_CIC_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_NAEA_CIC_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NAEA-CIC */
typedef OCTET_STRING_t	 NAEA_CIC_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NAEA_CIC;
asn_struct_free_f NAEA_CIC_free;
asn_struct_print_f NAEA_CIC_print;
asn_constr_check_f NAEA_CIC_constraint;
ber_type_decoder_f NAEA_CIC_decode_ber;
der_type_encoder_f NAEA_CIC_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f NAEA_CIC_decode_xer;
xer_type_encoder_f NAEA_CIC_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _NAEA_CIC_H_ */
