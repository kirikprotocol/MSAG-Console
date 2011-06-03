#ifndef	_GeographicalInformation_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_GeographicalInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GeographicalInformation */
typedef OCTET_STRING_t	 GeographicalInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GeographicalInformation;
asn_struct_free_f GeographicalInformation_free;
asn_struct_print_f GeographicalInformation_print;
asn_constr_check_f GeographicalInformation_constraint;
ber_type_decoder_f GeographicalInformation_decode_ber;
der_type_encoder_f GeographicalInformation_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f GeographicalInformation_decode_xer;
xer_type_encoder_f GeographicalInformation_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _GeographicalInformation_H_ */
