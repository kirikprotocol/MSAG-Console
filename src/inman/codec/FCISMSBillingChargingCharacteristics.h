#ifndef	_FCISMSBillingChargingCharacteristics_H_
#define	_FCISMSBillingChargingCharacteristics_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FCISMSBillingChargingCharacteristics */
typedef OCTET_STRING_t	 FCISMSBillingChargingCharacteristics_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_FCISMSBillingChargingCharacteristics;
asn_struct_free_f FCISMSBillingChargingCharacteristics_free;
asn_struct_print_f FCISMSBillingChargingCharacteristics_print;
asn_constr_check_f FCISMSBillingChargingCharacteristics_constraint;
ber_type_decoder_f FCISMSBillingChargingCharacteristics_decode_ber;
der_type_encoder_f FCISMSBillingChargingCharacteristics_encode_der;
xer_type_decoder_f FCISMSBillingChargingCharacteristics_decode_xer;
xer_type_encoder_f FCISMSBillingChargingCharacteristics_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _FCISMSBillingChargingCharacteristics_H_ */
