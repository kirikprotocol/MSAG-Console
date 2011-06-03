#ifndef	_ForwardingOptions_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ForwardingOptions_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ForwardingOptions */
typedef OCTET_STRING_t	 ForwardingOptions_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ForwardingOptions;
asn_struct_free_f ForwardingOptions_free;
asn_struct_print_f ForwardingOptions_print;
asn_constr_check_f ForwardingOptions_constraint;
ber_type_decoder_f ForwardingOptions_decode_ber;
der_type_encoder_f ForwardingOptions_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ForwardingOptions_decode_xer;
xer_type_encoder_f ForwardingOptions_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ForwardingOptions_H_ */
