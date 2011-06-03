#ifndef	_CallReferenceNumber_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CallReferenceNumber_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CallReferenceNumber */
typedef OCTET_STRING_t	 CallReferenceNumber_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CallReferenceNumber;
asn_struct_free_f CallReferenceNumber_free;
asn_struct_print_f CallReferenceNumber_print;
asn_constr_check_f CallReferenceNumber_constraint;
ber_type_decoder_f CallReferenceNumber_decode_ber;
der_type_encoder_f CallReferenceNumber_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f CallReferenceNumber_decode_xer;
xer_type_encoder_f CallReferenceNumber_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _CallReferenceNumber_H_ */
