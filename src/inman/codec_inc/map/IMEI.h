#ifndef	_IMEI_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_IMEI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <TBCD-STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IMEI */
typedef TBCD_STRING_t	 IMEI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IMEI;
asn_struct_free_f IMEI_free;
asn_struct_print_f IMEI_print;
asn_constr_check_f IMEI_constraint;
ber_type_decoder_f IMEI_decode_ber;
der_type_encoder_f IMEI_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f IMEI_decode_xer;
xer_type_encoder_f IMEI_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _IMEI_H_ */
