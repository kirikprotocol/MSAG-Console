#ifndef	_CriticalityType_H_
#define	_CriticalityType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CriticalityType {
	CriticalityType_ignore	= 0,
	CriticalityType_abort	= 1
} CriticalityType_e;

/* CriticalityType */
typedef long	 CriticalityType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CriticalityType;
asn_struct_free_f CriticalityType_free;
asn_struct_print_f CriticalityType_print;
asn_constr_check_f CriticalityType_constraint;
ber_type_decoder_f CriticalityType_decode_ber;
der_type_encoder_f CriticalityType_encode_der;
xer_type_decoder_f CriticalityType_decode_xer;
xer_type_encoder_f CriticalityType_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CriticalityType_H_ */
