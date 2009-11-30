#ifndef	_ContextId_H_
#define	_ContextId_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ContextId */
typedef long	 ContextId_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ContextId;
asn_struct_free_f ContextId_free;
asn_struct_print_f ContextId_print;
asn_constr_check_f ContextId_constraint;
ber_type_decoder_f ContextId_decode_ber;
der_type_encoder_f ContextId_encode_der;
xer_type_decoder_f ContextId_decode_xer;
xer_type_encoder_f ContextId_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ContextId_H_ */
