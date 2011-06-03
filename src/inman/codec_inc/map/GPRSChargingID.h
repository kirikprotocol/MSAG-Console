#ifndef	_GPRSChargingID_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_GPRSChargingID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPRSChargingID */
typedef OCTET_STRING_t	 GPRSChargingID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GPRSChargingID;
asn_struct_free_f GPRSChargingID_free;
asn_struct_print_f GPRSChargingID_print;
asn_constr_check_f GPRSChargingID_constraint;
ber_type_decoder_f GPRSChargingID_decode_ber;
der_type_encoder_f GPRSChargingID_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f GPRSChargingID_decode_xer;
xer_type_encoder_f GPRSChargingID_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _GPRSChargingID_H_ */
