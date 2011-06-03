#ifndef	_Ext2_QoS_Subscribed_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_Ext2_QoS_Subscribed_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ext2-QoS-Subscribed */
typedef OCTET_STRING_t	 Ext2_QoS_Subscribed_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Ext2_QoS_Subscribed;
asn_struct_free_f Ext2_QoS_Subscribed_free;
asn_struct_print_f Ext2_QoS_Subscribed_print;
asn_constr_check_f Ext2_QoS_Subscribed_constraint;
ber_type_decoder_f Ext2_QoS_Subscribed_decode_ber;
der_type_encoder_f Ext2_QoS_Subscribed_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f Ext2_QoS_Subscribed_decode_xer;
xer_type_encoder_f Ext2_QoS_Subscribed_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _Ext2_QoS_Subscribed_H_ */
