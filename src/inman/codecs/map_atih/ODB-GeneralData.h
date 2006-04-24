#ident "$Id$"

#ifndef	_ODB_GeneralData_H_
#define	_ODB_GeneralData_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ODB_GeneralData {
	ODB_GeneralData_allOG_CallsBarred	= 0,
	ODB_GeneralData_internationalOGCallsBarred	= 1,
	ODB_GeneralData_internationalOGCallsNotToHPLMN_CountryBarred	= 2,
	ODB_GeneralData_interzonalOGCallsBarred	= 6,
	ODB_GeneralData_interzonalOGCallsNotToHPLMN_CountryBarred	= 7,
	ODB_GeneralData_interzonalOGCallsAndInternationalOGCallsNotToHPLMN_CountryBarred	= 8,
	ODB_GeneralData_premiumRateInformationOGCallsBarred	= 3,
	ODB_GeneralData_premiumRateEntertainementOGCallsBarred	= 4,
	ODB_GeneralData_ss_AccessBarred	= 5,
	ODB_GeneralData_allECT_Barred	= 9,
	ODB_GeneralData_chargeableECT_Barred	= 10,
	ODB_GeneralData_internationalECT_Barred	= 11,
	ODB_GeneralData_interzonalECT_Barred	= 12,
	ODB_GeneralData_doublyChargeableECT_Barred	= 13,
	ODB_GeneralData_multipleECT_Barred	= 14,
	ODB_GeneralData_allPacketOrientedServicesBarred	= 15,
	ODB_GeneralData_roamerAccessToHPLMN_AP_Barred	= 16,
	ODB_GeneralData_roamerAccessToVPLMN_AP_Barred	= 17,
	ODB_GeneralData_roamingOutsidePLMNOG_CallsBarred	= 18,
	ODB_GeneralData_allIC_CallsBarred	= 19,
	ODB_GeneralData_roamingOutsidePLMNIC_CallsBarred	= 20,
	ODB_GeneralData_roamingOutsidePLMNICountryIC_CallsBarred	= 21,
	ODB_GeneralData_roamingOutsidePLMN_Barred	= 22,
	ODB_GeneralData_roamingOutsidePLMN_CountryBarred	= 23,
	ODB_GeneralData_registrationAllCF_Barred	= 24,
	ODB_GeneralData_registrationCFNotToHPLMN_Barred	= 25,
	ODB_GeneralData_registrationInterzonalCF_Barred	= 26,
	ODB_GeneralData_registrationInterzonalCFNotToHPLMN_Barred	= 27,
	ODB_GeneralData_registrationInternationalCF_Barred	= 28
} ODB_GeneralData_e;

/* ODB-GeneralData */
typedef BIT_STRING_t	 ODB_GeneralData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ODB_GeneralData;
asn_struct_free_f ODB_GeneralData_free;
asn_struct_print_f ODB_GeneralData_print;
asn_constr_check_f ODB_GeneralData_constraint;
ber_type_decoder_f ODB_GeneralData_decode_ber;
der_type_encoder_f ODB_GeneralData_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ODB_GeneralData_decode_xer;
xer_type_encoder_f ODB_GeneralData_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ODB_GeneralData_H_ */
