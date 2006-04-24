#ident "$Id$"

#ifndef	_RequestedCAMEL_SubscriptionInfo_H_
#define	_RequestedCAMEL_SubscriptionInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RequestedCAMEL_SubscriptionInfo {
	RequestedCAMEL_SubscriptionInfo_o_CSI	= 0,
	RequestedCAMEL_SubscriptionInfo_t_CSI	= 1,
	RequestedCAMEL_SubscriptionInfo_vt_CSI	= 2,
	RequestedCAMEL_SubscriptionInfo_tif_CSI	= 3,
	RequestedCAMEL_SubscriptionInfo_gprs_CSI	= 4,
	RequestedCAMEL_SubscriptionInfo_mo_sms_CSI	= 5,
	RequestedCAMEL_SubscriptionInfo_ss_CSI	= 6,
	RequestedCAMEL_SubscriptionInfo_m_CSI	= 7,
	RequestedCAMEL_SubscriptionInfo_d_csi	= 8
} RequestedCAMEL_SubscriptionInfo_e;

/* RequestedCAMEL-SubscriptionInfo */
typedef long	 RequestedCAMEL_SubscriptionInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RequestedCAMEL_SubscriptionInfo;
asn_struct_free_f RequestedCAMEL_SubscriptionInfo_free;
asn_struct_print_f RequestedCAMEL_SubscriptionInfo_print;
asn_constr_check_f RequestedCAMEL_SubscriptionInfo_constraint;
ber_type_decoder_f RequestedCAMEL_SubscriptionInfo_decode_ber;
der_type_encoder_f RequestedCAMEL_SubscriptionInfo_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f RequestedCAMEL_SubscriptionInfo_decode_xer;
xer_type_encoder_f RequestedCAMEL_SubscriptionInfo_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _RequestedCAMEL_SubscriptionInfo_H_ */
