#ifndef	_MSNetworkCapability_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_MSNetworkCapability_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MSNetworkCapability */
typedef OCTET_STRING_t	 MSNetworkCapability_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MSNetworkCapability;
asn_struct_free_f MSNetworkCapability_free;
asn_struct_print_f MSNetworkCapability_print;
asn_constr_check_f MSNetworkCapability_constraint;
ber_type_decoder_f MSNetworkCapability_decode_ber;
der_type_encoder_f MSNetworkCapability_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f MSNetworkCapability_decode_xer;
xer_type_encoder_f MSNetworkCapability_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _MSNetworkCapability_H_ */
