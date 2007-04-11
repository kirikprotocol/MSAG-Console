#ifndef	_UnknownSubscriberDiagnostic_H_
#define	_UnknownSubscriberDiagnostic_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum UnknownSubscriberDiagnostic {
	UnknownSubscriberDiagnostic_imsiUnknown	= 0,
	UnknownSubscriberDiagnostic_gprsSubscriptionUnknown	= 1,
	/*
	 * Enumeration is extensible
	 */
	UnknownSubscriberDiagnostic_npdbMismatch	= 2
} e_UnknownSubscriberDiagnostic;

/* UnknownSubscriberDiagnostic */
typedef long	 UnknownSubscriberDiagnostic_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UnknownSubscriberDiagnostic;
asn_struct_free_f UnknownSubscriberDiagnostic_free;
asn_struct_print_f UnknownSubscriberDiagnostic_print;
asn_constr_check_f UnknownSubscriberDiagnostic_constraint;
ber_type_decoder_f UnknownSubscriberDiagnostic_decode_ber;
der_type_encoder_f UnknownSubscriberDiagnostic_encode_der;
xer_type_decoder_f UnknownSubscriberDiagnostic_decode_xer;
xer_type_encoder_f UnknownSubscriberDiagnostic_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _UnknownSubscriberDiagnostic_H_ */
