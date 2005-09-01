#ifndef _InvokeProblem_H_
#define _InvokeProblem_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <asn_application.h>

#include <NativeInteger.h>

typedef enum InvokeProblem {
  InvokeProblem_duplicateInvokeID = 0,
  InvokeProblem_unrecognizedOperation = 1,
  InvokeProblem_mistypedParameter = 2,
  InvokeProblem_resourceLimitation  = 3,
  InvokeProblem_initiatingRelease = 4,
  InvokeProblem_unrecognizedLinkedID  = 5,
  InvokeProblem_linkedResponceUnexpected  = 6,
  InvokeProblem_unexpectedLinkedOperation = 7
} InvokeProblem_e;


typedef int  InvokeProblem_t;

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

#endif  /* _InvokeProblem_H_ */
