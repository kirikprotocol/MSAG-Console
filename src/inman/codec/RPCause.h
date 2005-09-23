#ifndef	_RPCause_H_
#define	_RPCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RPCause */
typedef OCTET_STRING_t	 RPCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RPCause;
asn_struct_free_f RPCause_free;
asn_struct_print_f RPCause_print;
asn_constr_check_f RPCause_constraint;
ber_type_decoder_f RPCause_decode_ber;
der_type_encoder_f RPCause_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f RPCause_decode_xer;
xer_type_encoder_f RPCause_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _RPCause_H_ */
