#ifndef	_AbsentSubscriberDiagnosticSM_H_
#define	_AbsentSubscriberDiagnosticSM_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AbsentSubscriberDiagnosticSM */
typedef long	 AbsentSubscriberDiagnosticSM_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AbsentSubscriberDiagnosticSM;
asn_struct_free_f AbsentSubscriberDiagnosticSM_free;
asn_struct_print_f AbsentSubscriberDiagnosticSM_print;
asn_constr_check_f AbsentSubscriberDiagnosticSM_constraint;
ber_type_decoder_f AbsentSubscriberDiagnosticSM_decode_ber;
der_type_encoder_f AbsentSubscriberDiagnosticSM_encode_der;
xer_type_decoder_f AbsentSubscriberDiagnosticSM_decode_xer;
xer_type_encoder_f AbsentSubscriberDiagnosticSM_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _AbsentSubscriberDiagnosticSM_H_ */
