#ifndef	_ForwardingOptions_v1_H_
#define	_ForwardingOptions_v1_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ForwardingOptions-v1 */
typedef OCTET_STRING_t	 ForwardingOptions_v1_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ForwardingOptions_v1;
asn_struct_free_f ForwardingOptions_v1_free;
asn_struct_print_f ForwardingOptions_v1_print;
asn_constr_check_f ForwardingOptions_v1_constraint;
ber_type_decoder_f ForwardingOptions_v1_decode_ber;
der_type_encoder_f ForwardingOptions_v1_encode_der;
xer_type_decoder_f ForwardingOptions_v1_decode_xer;
xer_type_encoder_f ForwardingOptions_v1_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ForwardingOptions_v1_H_ */
