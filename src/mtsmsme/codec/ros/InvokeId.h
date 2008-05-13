#ifndef	_InvokeId_H_
#define	_InvokeId_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* InvokeId */
typedef long	 InvokeId_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InvokeId;
asn_struct_free_f InvokeId_free;
asn_struct_print_f InvokeId_print;
asn_constr_check_f InvokeId_constraint;
ber_type_decoder_f InvokeId_decode_ber;
der_type_encoder_f InvokeId_encode_der;
xer_type_decoder_f InvokeId_decode_xer;
xer_type_encoder_f InvokeId_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _InvokeId_H_ */
