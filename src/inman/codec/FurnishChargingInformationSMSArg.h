#ifndef	_FurnishChargingInformationSMSArg_H_
#define	_FurnishChargingInformationSMSArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <FCISMSBillingChargingCharacteristics.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FurnishChargingInformationSMSArg */
typedef FCISMSBillingChargingCharacteristics_t	 FurnishChargingInformationSMSArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_FurnishChargingInformationSMSArg;
asn_struct_free_f FurnishChargingInformationSMSArg_free;
asn_struct_print_f FurnishChargingInformationSMSArg_print;
asn_constr_check_f FurnishChargingInformationSMSArg_constraint;
ber_type_decoder_f FurnishChargingInformationSMSArg_decode_ber;
der_type_encoder_f FurnishChargingInformationSMSArg_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f FurnishChargingInformationSMSArg_decode_xer;
xer_type_encoder_f FurnishChargingInformationSMSArg_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _FurnishChargingInformationSMSArg_H_ */
