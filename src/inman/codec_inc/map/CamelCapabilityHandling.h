#ifndef	_CamelCapabilityHandling_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_CamelCapabilityHandling_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CamelCapabilityHandling */
typedef long	 CamelCapabilityHandling_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CamelCapabilityHandling;
asn_struct_free_f CamelCapabilityHandling_free;
asn_struct_print_f CamelCapabilityHandling_print;
asn_constr_check_f CamelCapabilityHandling_constraint;
ber_type_decoder_f CamelCapabilityHandling_decode_ber;
der_type_encoder_f CamelCapabilityHandling_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f CamelCapabilityHandling_decode_xer;
xer_type_encoder_f CamelCapabilityHandling_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _CamelCapabilityHandling_H_ */
