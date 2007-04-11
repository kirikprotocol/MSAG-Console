#ifndef	_InvokeIdType_H_
#define	_InvokeIdType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* InvokeIdType */
typedef long	 InvokeIdType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InvokeIdType;
asn_struct_free_f InvokeIdType_free;
asn_struct_print_f InvokeIdType_print;
asn_constr_check_f InvokeIdType_constraint;
ber_type_decoder_f InvokeIdType_decode_ber;
der_type_encoder_f InvokeIdType_encode_der;
xer_type_decoder_f InvokeIdType_decode_xer;
xer_type_encoder_f InvokeIdType_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _InvokeIdType_H_ */
