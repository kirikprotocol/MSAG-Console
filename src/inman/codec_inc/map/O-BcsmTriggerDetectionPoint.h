#ifndef	_O_BcsmTriggerDetectionPoint_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_O_BcsmTriggerDetectionPoint_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum O_BcsmTriggerDetectionPoint {
	O_BcsmTriggerDetectionPoint_collectedInfo	= 2,
	/*
	 * Enumeration is extensible
	 */
	O_BcsmTriggerDetectionPoint_routeSelectFailure	= 4
} O_BcsmTriggerDetectionPoint_e;

/* O-BcsmTriggerDetectionPoint */
typedef long	 O_BcsmTriggerDetectionPoint_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_O_BcsmTriggerDetectionPoint;
asn_struct_free_f O_BcsmTriggerDetectionPoint_free;
asn_struct_print_f O_BcsmTriggerDetectionPoint_print;
asn_constr_check_f O_BcsmTriggerDetectionPoint_constraint;
ber_type_decoder_f O_BcsmTriggerDetectionPoint_decode_ber;
der_type_encoder_f O_BcsmTriggerDetectionPoint_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f O_BcsmTriggerDetectionPoint_decode_xer;
xer_type_encoder_f O_BcsmTriggerDetectionPoint_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _O_BcsmTriggerDetectionPoint_H_ */
