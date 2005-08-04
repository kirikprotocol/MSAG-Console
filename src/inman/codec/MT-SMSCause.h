#ifndef	_MT_SMSCause_H_
#define	_MT_SMSCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MT-SMSCause */
typedef OCTET_STRING_t	 MT_SMSCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MT_SMSCause;
asn_struct_free_f MT_SMSCause_free;
asn_struct_print_f MT_SMSCause_print;
asn_constr_check_f MT_SMSCause_constraint;
ber_type_decoder_f MT_SMSCause_decode_ber;
der_type_encoder_f MT_SMSCause_encode_der;
xer_type_decoder_f MT_SMSCause_decode_xer;
xer_type_encoder_f MT_SMSCause_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _MT_SMSCause_H_ */
