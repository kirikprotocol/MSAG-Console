#ifndef	_CallTypeCriteria_H_
#define	_CallTypeCriteria_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CallTypeCriteria {
	CallTypeCriteria_forwarded	= 0,
	CallTypeCriteria_notForwarded	= 1
} e_CallTypeCriteria;

/* CallTypeCriteria */
typedef long	 CallTypeCriteria_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CallTypeCriteria;
asn_struct_free_f CallTypeCriteria_free;
asn_struct_print_f CallTypeCriteria_print;
asn_constr_check_f CallTypeCriteria_constraint;
ber_type_decoder_f CallTypeCriteria_decode_ber;
der_type_encoder_f CallTypeCriteria_encode_der;
xer_type_decoder_f CallTypeCriteria_decode_xer;
xer_type_encoder_f CallTypeCriteria_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CallTypeCriteria_H_ */
