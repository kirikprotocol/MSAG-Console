#ifndef	_InsertSubscriberDataArg_H_
#define	_InsertSubscriberDataArg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* InsertSubscriberDataArg */
typedef OCTET_STRING_t	 InsertSubscriberDataArg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_InsertSubscriberDataArg;
asn_struct_free_f InsertSubscriberDataArg_free;
asn_struct_print_f InsertSubscriberDataArg_print;
asn_constr_check_f InsertSubscriberDataArg_constraint;
ber_type_decoder_f InsertSubscriberDataArg_decode_ber;
der_type_encoder_f InsertSubscriberDataArg_encode_der;
xer_type_decoder_f InsertSubscriberDataArg_decode_xer;
xer_type_encoder_f InsertSubscriberDataArg_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _InsertSubscriberDataArg_H_ */
