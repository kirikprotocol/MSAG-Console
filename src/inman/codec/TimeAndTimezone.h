#ifndef	_TimeAndTimezone_H_
#define	_TimeAndTimezone_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TimeAndTimezone */
typedef OCTET_STRING_t	 TimeAndTimezone_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TimeAndTimezone;
asn_struct_free_f TimeAndTimezone_free;
asn_struct_print_f TimeAndTimezone_print;
asn_constr_check_f TimeAndTimezone_constraint;
ber_type_decoder_f TimeAndTimezone_decode_ber;
der_type_encoder_f TimeAndTimezone_encode_der;
xer_type_decoder_f TimeAndTimezone_decode_xer;
xer_type_encoder_f TimeAndTimezone_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _TimeAndTimezone_H_ */
