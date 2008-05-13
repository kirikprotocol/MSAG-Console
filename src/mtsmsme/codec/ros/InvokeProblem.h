#ifndef	_InvokeProblem_H_
#define	_InvokeProblem_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum InvokeProblem {
	InvokeProblem_duplicateInvocation	= 0,
	InvokeProblem_unrecognizedOperation	= 1,
	InvokeProblem_mistypedArgument	= 2,
	InvokeProblem_resourceLimitation	= 3,
	InvokeProblem_releaseInProgress	= 4,
	InvokeProblem_unrecognizedLinkedId	= 5,
	InvokeProblem_linkedResponseUnexpected	= 6,
	InvokeProblem_unexpectedLinkedOperation	= 7
} e_InvokeProblem;

/* InvokeProblem */
typedef long	 InvokeProblem_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InvokeProblem;
asn_struct_free_f InvokeProblem_free;
asn_struct_print_f InvokeProblem_print;
asn_constr_check_f InvokeProblem_constraint;
ber_type_decoder_f InvokeProblem_decode_ber;
der_type_encoder_f InvokeProblem_encode_der;
xer_type_decoder_f InvokeProblem_decode_xer;
xer_type_encoder_f InvokeProblem_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _InvokeProblem_H_ */
