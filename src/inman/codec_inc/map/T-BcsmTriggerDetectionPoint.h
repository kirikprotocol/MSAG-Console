#ifndef	_T_BcsmTriggerDetectionPoint_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_T_BcsmTriggerDetectionPoint_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum T_BcsmTriggerDetectionPoint {
	T_BcsmTriggerDetectionPoint_termAttemptAuthorized	= 12,
	/*
	 * Enumeration is extensible
	 */
	T_BcsmTriggerDetectionPoint_tBusy	= 13,
	T_BcsmTriggerDetectionPoint_tNoAnswer	= 14
} T_BcsmTriggerDetectionPoint_e;

/* T-BcsmTriggerDetectionPoint */
typedef long	 T_BcsmTriggerDetectionPoint_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_T_BcsmTriggerDetectionPoint;
asn_struct_free_f T_BcsmTriggerDetectionPoint_free;
asn_struct_print_f T_BcsmTriggerDetectionPoint_print;
asn_constr_check_f T_BcsmTriggerDetectionPoint_constraint;
ber_type_decoder_f T_BcsmTriggerDetectionPoint_decode_ber;
der_type_encoder_f T_BcsmTriggerDetectionPoint_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f T_BcsmTriggerDetectionPoint_decode_xer;
xer_type_encoder_f T_BcsmTriggerDetectionPoint_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _T_BcsmTriggerDetectionPoint_H_ */
