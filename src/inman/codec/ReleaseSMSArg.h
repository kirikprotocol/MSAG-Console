#ifndef	_ReleaseSMSArg_H_
#define	_ReleaseSMSArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <RPCause.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReleaseSMSArg */
typedef RPCause_t	 ReleaseSMSArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ReleaseSMSArg;
asn_struct_free_f ReleaseSMSArg_free;
asn_struct_print_f ReleaseSMSArg_print;
asn_constr_check_f ReleaseSMSArg_constraint;
ber_type_decoder_f ReleaseSMSArg_decode_ber;
der_type_encoder_f ReleaseSMSArg_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ReleaseSMSArg_decode_xer;
xer_type_encoder_f ReleaseSMSArg_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ReleaseSMSArg_H_ */
