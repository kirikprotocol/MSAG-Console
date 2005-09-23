#ifndef	_TPProtocolIdentifier_H_
#define	_TPProtocolIdentifier_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TPProtocolIdentifier */
typedef OCTET_STRING_t	 TPProtocolIdentifier_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TPProtocolIdentifier;
asn_struct_free_f TPProtocolIdentifier_free;
asn_struct_print_f TPProtocolIdentifier_print;
asn_constr_check_f TPProtocolIdentifier_constraint;
ber_type_decoder_f TPProtocolIdentifier_decode_ber;
der_type_encoder_f TPProtocolIdentifier_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f TPProtocolIdentifier_decode_xer;
xer_type_encoder_f TPProtocolIdentifier_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _TPProtocolIdentifier_H_ */
