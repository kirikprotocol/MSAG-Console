#ifndef _ReturnErrorProblem_H_
#define _ReturnErrorProblem_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <NativeInteger.h>

typedef enum ReturnErrorProblem {
  ReturnErrorProblem_unrecognizedInvokeID = 0,
  ReturnErrorProblem_returnErrorUnexpected  = 1,
  ReturnErrorProblem_unrecognizedError  = 2,
  ReturnErrorProblem_unexpectedError  = 3,
  ReturnErrorProblem_mistypedParameter  = 4
} ReturnErrorProblem_e;


typedef int  ReturnErrorProblem_t;

extern asn_TYPE_descriptor_t asn_DEF_ReturnErrorProblem;
asn_struct_free_f ReturnErrorProblem_free;
asn_struct_print_f ReturnErrorProblem_print;
asn_constr_check_f ReturnErrorProblem_constraint;
ber_type_decoder_f ReturnErrorProblem_decode_ber;
der_type_encoder_f ReturnErrorProblem_encode_der;
xer_type_decoder_f ReturnErrorProblem_decode_xer;
xer_type_encoder_f ReturnErrorProblem_encode_xer;

#ifdef __cplusplus
}
#endif

#endif  /* _ReturnErrorProblem_H_ */
