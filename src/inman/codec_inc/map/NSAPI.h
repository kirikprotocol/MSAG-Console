#ifndef	_NSAPI_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_NSAPI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NSAPI */
typedef long	 NSAPI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NSAPI;
asn_struct_free_f NSAPI_free;
asn_struct_print_f NSAPI_print;
asn_constr_check_f NSAPI_constraint;
ber_type_decoder_f NSAPI_decode_ber;
der_type_encoder_f NSAPI_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f NSAPI_decode_xer;
xer_type_encoder_f NSAPI_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _NSAPI_H_ */
