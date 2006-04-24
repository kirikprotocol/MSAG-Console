#ident "$Id$"

#ifndef	_MT_SMS_TPDU_Type_H_
#define	_MT_SMS_TPDU_Type_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum MT_SMS_TPDU_Type {
	MT_SMS_TPDU_Type_sms_DELIVER	= 0,
	MT_SMS_TPDU_Type_sms_SUBMIT_REPORT	= 1,
	MT_SMS_TPDU_Type_sms_STATUS_REPORT	= 2
	/*
	 * Enumeration is extensible
	 */
} MT_SMS_TPDU_Type_e;

/* MT-SMS-TPDU-Type */
typedef long	 MT_SMS_TPDU_Type_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MT_SMS_TPDU_Type;
asn_struct_free_f MT_SMS_TPDU_Type_free;
asn_struct_print_f MT_SMS_TPDU_Type_print;
asn_constr_check_f MT_SMS_TPDU_Type_constraint;
ber_type_decoder_f MT_SMS_TPDU_Type_decode_ber;
der_type_encoder_f MT_SMS_TPDU_Type_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f MT_SMS_TPDU_Type_decode_xer;
xer_type_encoder_f MT_SMS_TPDU_Type_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _MT_SMS_TPDU_Type_H_ */
