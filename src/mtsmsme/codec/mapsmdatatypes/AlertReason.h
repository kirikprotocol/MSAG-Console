#ifndef	_AlertReason_H_
#define	_AlertReason_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AlertReason {
	AlertReason_ms_Present	= 0,
	AlertReason_memoryAvailable	= 1
} e_AlertReason;

/* AlertReason */
typedef long	 AlertReason_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AlertReason;
asn_struct_free_f AlertReason_free;
asn_struct_print_f AlertReason_print;
asn_constr_check_f AlertReason_constraint;
ber_type_decoder_f AlertReason_decode_ber;
der_type_encoder_f AlertReason_encode_der;
xer_type_decoder_f AlertReason_decode_xer;
xer_type_encoder_f AlertReason_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _AlertReason_H_ */
