#ifndef	_ProtocolId_H_
#define	_ProtocolId_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ProtocolId {
	ProtocolId_gsm_0408	= 1,
	ProtocolId_gsm_0806	= 2,
	ProtocolId_gsm_BSSMAP	= 3,
	ProtocolId_ets_300102_1	= 4
} e_ProtocolId;

/* ProtocolId */
typedef long	 ProtocolId_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ProtocolId;
asn_struct_free_f ProtocolId_free;
asn_struct_print_f ProtocolId_print;
asn_constr_check_f ProtocolId_constraint;
ber_type_decoder_f ProtocolId_decode_ber;
der_type_encoder_f ProtocolId_encode_der;
xer_type_decoder_f ProtocolId_decode_xer;
xer_type_encoder_f ProtocolId_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ProtocolId_H_ */
