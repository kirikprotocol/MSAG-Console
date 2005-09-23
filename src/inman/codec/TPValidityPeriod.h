#ifndef	_TPValidityPeriod_H_
#define	_TPValidityPeriod_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TPValidityPeriod */
typedef OCTET_STRING_t	 TPValidityPeriod_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TPValidityPeriod;
asn_struct_free_f TPValidityPeriod_free;
asn_struct_print_f TPValidityPeriod_print;
asn_constr_check_f TPValidityPeriod_constraint;
ber_type_decoder_f TPValidityPeriod_decode_ber;
der_type_encoder_f TPValidityPeriod_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f TPValidityPeriod_decode_xer;
xer_type_encoder_f TPValidityPeriod_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _TPValidityPeriod_H_ */
