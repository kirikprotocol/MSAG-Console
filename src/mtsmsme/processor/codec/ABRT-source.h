#ifndef	_ABRT_source_H_
#define	_ABRT_source_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ABRT_source {
	ABRT_source_dialogue_service_user	= 0,
	ABRT_source_dialogue_service_provider	= 1
} e_ABRT_source;

/* ABRT-source */
typedef long	 ABRT_source_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ABRT_source;
asn_struct_free_f ABRT_source_free;
asn_struct_print_f ABRT_source_print;
asn_constr_check_f ABRT_source_constraint;
ber_type_decoder_f ABRT_source_decode_ber;
der_type_encoder_f ABRT_source_encode_der;
xer_type_decoder_f ABRT_source_decode_xer;
xer_type_encoder_f ABRT_source_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ABRT_source_H_ */
