#ifndef	_ReturnResultProblem_H_
#define	_ReturnResultProblem_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ReturnResultProblem {
	ReturnResultProblem_unrecognizedInvocation	= 0,
	ReturnResultProblem_resultResponseUnexpected	= 1,
	ReturnResultProblem_mistypedResult	= 2
} e_ReturnResultProblem;

/* ReturnResultProblem */
typedef long	 ReturnResultProblem_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReturnResultProblem;
asn_struct_free_f ReturnResultProblem_free;
asn_struct_print_f ReturnResultProblem_print;
asn_constr_check_f ReturnResultProblem_constraint;
ber_type_decoder_f ReturnResultProblem_decode_ber;
der_type_encoder_f ReturnResultProblem_encode_der;
xer_type_decoder_f ReturnResultProblem_decode_xer;
xer_type_encoder_f ReturnResultProblem_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ReturnResultProblem_H_ */
