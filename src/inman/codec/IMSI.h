#ifndef	_IMSI_H_
#define	_IMSI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <TBCD-STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IMSI */
typedef TBCD_STRING_t	 IMSI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IMSI;
asn_struct_free_f IMSI_free;
asn_struct_print_f IMSI_print;
asn_constr_check_f IMSI_constraint;
ber_type_decoder_f IMSI_decode_ber;
der_type_encoder_f IMSI_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f IMSI_decode_xer;
xer_type_encoder_f IMSI_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _IMSI_H_ */
