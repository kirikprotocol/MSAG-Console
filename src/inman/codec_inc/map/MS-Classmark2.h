#ident "$Id$"

#ifndef	_MS_Classmark2_H_
#define	_MS_Classmark2_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MS-Classmark2 */
typedef OCTET_STRING_t	 MS_Classmark2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MS_Classmark2;
asn_struct_free_f MS_Classmark2_free;
asn_struct_print_f MS_Classmark2_print;
asn_constr_check_f MS_Classmark2_constraint;
ber_type_decoder_f MS_Classmark2_decode_ber;
der_type_encoder_f MS_Classmark2_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f MS_Classmark2_decode_xer;
xer_type_encoder_f MS_Classmark2_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _MS_Classmark2_H_ */
