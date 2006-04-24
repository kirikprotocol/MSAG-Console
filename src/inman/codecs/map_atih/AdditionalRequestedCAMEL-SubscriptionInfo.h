#ident "$Id$"

#ifndef	_AdditionalRequestedCAMEL_SubscriptionInfo_H_
#define	_AdditionalRequestedCAMEL_SubscriptionInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AdditionalRequestedCAMEL_SubscriptionInfo {
	AdditionalRequestedCAMEL_SubscriptionInfo_mt_sms_CSI	= 0,
	AdditionalRequestedCAMEL_SubscriptionInfo_mg_csi	= 1,
	AdditionalRequestedCAMEL_SubscriptionInfo_o_IM_CSI	= 2,
	AdditionalRequestedCAMEL_SubscriptionInfo_d_IM_CSI	= 3,
	AdditionalRequestedCAMEL_SubscriptionInfo_vt_IM_CSI	= 4
	/*
	 * Enumeration is extensible
	 */
} AdditionalRequestedCAMEL_SubscriptionInfo_e;

/* AdditionalRequestedCAMEL-SubscriptionInfo */
typedef long	 AdditionalRequestedCAMEL_SubscriptionInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AdditionalRequestedCAMEL_SubscriptionInfo;
asn_struct_free_f AdditionalRequestedCAMEL_SubscriptionInfo_free;
asn_struct_print_f AdditionalRequestedCAMEL_SubscriptionInfo_print;
asn_constr_check_f AdditionalRequestedCAMEL_SubscriptionInfo_constraint;
ber_type_decoder_f AdditionalRequestedCAMEL_SubscriptionInfo_decode_ber;
der_type_encoder_f AdditionalRequestedCAMEL_SubscriptionInfo_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f AdditionalRequestedCAMEL_SubscriptionInfo_decode_xer;
xer_type_encoder_f AdditionalRequestedCAMEL_SubscriptionInfo_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _AdditionalRequestedCAMEL_SubscriptionInfo_H_ */
