#ident "$Id$"

#ifndef	_GPRS_TriggerDetectionPoint_H_
#define	_GPRS_TriggerDetectionPoint_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum GPRS_TriggerDetectionPoint {
	GPRS_TriggerDetectionPoint_attach	= 1,
	GPRS_TriggerDetectionPoint_attachChangeOfPosition	= 2,
	GPRS_TriggerDetectionPoint_pdp_ContextEstablishment	= 11,
	GPRS_TriggerDetectionPoint_pdp_ContextEstablishmentAcknowledgement	= 12,
	GPRS_TriggerDetectionPoint_pdp_ContextChangeOfPosition	= 14
	/*
	 * Enumeration is extensible
	 */
} GPRS_TriggerDetectionPoint_e;

/* GPRS-TriggerDetectionPoint */
typedef long	 GPRS_TriggerDetectionPoint_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GPRS_TriggerDetectionPoint;
asn_struct_free_f GPRS_TriggerDetectionPoint_free;
asn_struct_print_f GPRS_TriggerDetectionPoint_print;
asn_constr_check_f GPRS_TriggerDetectionPoint_constraint;
ber_type_decoder_f GPRS_TriggerDetectionPoint_decode_ber;
der_type_encoder_f GPRS_TriggerDetectionPoint_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f GPRS_TriggerDetectionPoint_decode_xer;
xer_type_encoder_f GPRS_TriggerDetectionPoint_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _GPRS_TriggerDetectionPoint_H_ */
