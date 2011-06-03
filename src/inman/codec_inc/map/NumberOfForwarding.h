#ifndef	_NumberOfForwarding_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_NumberOfForwarding_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NumberOfForwarding */
typedef long	 NumberOfForwarding_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NumberOfForwarding;
asn_struct_free_f NumberOfForwarding_free;
asn_struct_print_f NumberOfForwarding_print;
asn_constr_check_f NumberOfForwarding_constraint;
ber_type_decoder_f NumberOfForwarding_decode_ber;
der_type_encoder_f NumberOfForwarding_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f NumberOfForwarding_decode_xer;
xer_type_encoder_f NumberOfForwarding_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _NumberOfForwarding_H_ */
