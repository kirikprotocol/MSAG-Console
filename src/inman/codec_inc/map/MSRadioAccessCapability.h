#ifndef	_MSRadioAccessCapability_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_MSRadioAccessCapability_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MSRadioAccessCapability */
typedef OCTET_STRING_t	 MSRadioAccessCapability_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MSRadioAccessCapability;
asn_struct_free_f MSRadioAccessCapability_free;
asn_struct_print_f MSRadioAccessCapability_print;
asn_constr_check_f MSRadioAccessCapability_constraint;
ber_type_decoder_f MSRadioAccessCapability_decode_ber;
der_type_encoder_f MSRadioAccessCapability_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f MSRadioAccessCapability_decode_xer;
xer_type_encoder_f MSRadioAccessCapability_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _MSRadioAccessCapability_H_ */
