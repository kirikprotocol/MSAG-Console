#ifndef	_CUG_RejectCause_H_
#define	_CUG_RejectCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CUG_RejectCause {
	CUG_RejectCause_incomingCallsBarredWithinCUG	= 0,
	CUG_RejectCause_subscriberNotMemberOfCUG	= 1,
	CUG_RejectCause_requestedBasicServiceViolatesCUG_Constraints	= 5,
	CUG_RejectCause_calledPartySS_InteractionViolation	= 7
} e_CUG_RejectCause;

/* CUG-RejectCause */
typedef long	 CUG_RejectCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CUG_RejectCause;
asn_struct_free_f CUG_RejectCause_free;
asn_struct_print_f CUG_RejectCause_print;
asn_constr_check_f CUG_RejectCause_constraint;
ber_type_decoder_f CUG_RejectCause_decode_ber;
der_type_encoder_f CUG_RejectCause_encode_der;
xer_type_decoder_f CUG_RejectCause_decode_xer;
xer_type_encoder_f CUG_RejectCause_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CUG_RejectCause_H_ */
