#ifndef	_AgeOfLocationInformation_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_AgeOfLocationInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AgeOfLocationInformation */
typedef long	 AgeOfLocationInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AgeOfLocationInformation;
asn_struct_free_f AgeOfLocationInformation_free;
asn_struct_print_f AgeOfLocationInformation_print;
asn_constr_check_f AgeOfLocationInformation_constraint;
ber_type_decoder_f AgeOfLocationInformation_decode_ber;
der_type_encoder_f AgeOfLocationInformation_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f AgeOfLocationInformation_decode_xer;
xer_type_encoder_f AgeOfLocationInformation_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _AgeOfLocationInformation_H_ */
