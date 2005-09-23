#ifndef	_TPDataCodingScheme_H_
#define	_TPDataCodingScheme_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TPDataCodingScheme */
typedef OCTET_STRING_t	 TPDataCodingScheme_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TPDataCodingScheme;
asn_struct_free_f TPDataCodingScheme_free;
asn_struct_print_f TPDataCodingScheme_print;
asn_constr_check_f TPDataCodingScheme_constraint;
ber_type_decoder_f TPDataCodingScheme_decode_ber;
der_type_encoder_f TPDataCodingScheme_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f TPDataCodingScheme_decode_xer;
xer_type_encoder_f TPDataCodingScheme_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _TPDataCodingScheme_H_ */
