#ifndef	_USSD_DataCodingScheme_H_
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define	_USSD_DataCodingScheme_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* USSD-DataCodingScheme */
typedef OCTET_STRING_t	 USSD_DataCodingScheme_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_USSD_DataCodingScheme;
asn_struct_free_f USSD_DataCodingScheme_free;
asn_struct_print_f USSD_DataCodingScheme_print;
asn_constr_check_f USSD_DataCodingScheme_constraint;
ber_type_decoder_f USSD_DataCodingScheme_decode_ber;
der_type_encoder_f USSD_DataCodingScheme_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f USSD_DataCodingScheme_decode_xer;
xer_type_encoder_f USSD_DataCodingScheme_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _USSD_DataCodingScheme_H_ */
