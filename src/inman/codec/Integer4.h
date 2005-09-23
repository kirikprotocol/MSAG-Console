#ifndef	_Integer4_H_
#define	_Integer4_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Integer4 */
typedef long	 Integer4_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Integer4;
asn_struct_free_f Integer4_free;
asn_struct_print_f Integer4_print;
asn_constr_check_f Integer4_constraint;
ber_type_decoder_f Integer4_decode_ber;
der_type_encoder_f Integer4_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f Integer4_decode_xer;
xer_type_encoder_f Integer4_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _Integer4_H_ */
