#ifndef	_ChargingCharacteristics_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ChargingCharacteristics_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChargingCharacteristics */
typedef OCTET_STRING_t	 ChargingCharacteristics_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChargingCharacteristics;
asn_struct_free_f ChargingCharacteristics_free;
asn_struct_print_f ChargingCharacteristics_print;
asn_constr_check_f ChargingCharacteristics_constraint;
ber_type_decoder_f ChargingCharacteristics_decode_ber;
der_type_encoder_f ChargingCharacteristics_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ChargingCharacteristics_decode_xer;
xer_type_encoder_f ChargingCharacteristics_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ChargingCharacteristics_H_ */
