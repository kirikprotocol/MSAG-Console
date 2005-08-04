#ifndef	_MonitorMode_H_
#define	_MonitorMode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum MonitorMode {
	MonitorMode_interrupted	= 0,
	MonitorMode_notifyAndContinue	= 1,
	MonitorMode_transparent	= 2
} MonitorMode_e;

/* MonitorMode */
typedef long	 MonitorMode_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MonitorMode;
asn_struct_free_f MonitorMode_free;
asn_struct_print_f MonitorMode_print;
asn_constr_check_f MonitorMode_constraint;
ber_type_decoder_f MonitorMode_decode_ber;
der_type_encoder_f MonitorMode_encode_der;
xer_type_decoder_f MonitorMode_decode_xer;
xer_type_encoder_f MonitorMode_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _MonitorMode_H_ */
