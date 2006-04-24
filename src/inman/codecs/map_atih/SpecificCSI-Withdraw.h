#ident "$Id$"

#ifndef	_SpecificCSI_Withdraw_H_
#define	_SpecificCSI_Withdraw_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SpecificCSI_Withdraw {
	SpecificCSI_Withdraw_o_csi	= 0,
	SpecificCSI_Withdraw_ss_csi	= 1,
	SpecificCSI_Withdraw_tif_csi	= 2,
	SpecificCSI_Withdraw_d_csi	= 3,
	SpecificCSI_Withdraw_vt_csi	= 4,
	SpecificCSI_Withdraw_mo_sms_csi	= 5,
	SpecificCSI_Withdraw_m_csi	= 6,
	SpecificCSI_Withdraw_gprs_csi	= 7,
	SpecificCSI_Withdraw_t_csi	= 8,
	SpecificCSI_Withdraw_mt_sms_csi	= 9,
	SpecificCSI_Withdraw_mg_csi	= 10,
	SpecificCSI_Withdraw_o_IM_CSI	= 11,
	SpecificCSI_Withdraw_d_IM_CSI	= 12,
	SpecificCSI_Withdraw_vt_IM_CSI	= 13
} SpecificCSI_Withdraw_e;

/* SpecificCSI-Withdraw */
typedef BIT_STRING_t	 SpecificCSI_Withdraw_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SpecificCSI_Withdraw;
asn_struct_free_f SpecificCSI_Withdraw_free;
asn_struct_print_f SpecificCSI_Withdraw_print;
asn_constr_check_f SpecificCSI_Withdraw_constraint;
ber_type_decoder_f SpecificCSI_Withdraw_decode_ber;
der_type_encoder_f SpecificCSI_Withdraw_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f SpecificCSI_Withdraw_decode_xer;
xer_type_encoder_f SpecificCSI_Withdraw_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _SpecificCSI_Withdraw_H_ */
