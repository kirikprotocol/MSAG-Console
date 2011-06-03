#ifndef	_ServiceKey_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ServiceKey_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ServiceKey */
typedef long	 ServiceKey_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ServiceKey;
asn_struct_free_f ServiceKey_free;
asn_struct_print_f ServiceKey_print;
asn_constr_check_f ServiceKey_constraint;
ber_type_decoder_f ServiceKey_decode_ber;
der_type_encoder_f ServiceKey_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ServiceKey_decode_xer;
xer_type_encoder_f ServiceKey_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ServiceKey_H_ */
