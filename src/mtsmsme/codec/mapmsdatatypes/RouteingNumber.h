#ifndef	_RouteingNumber_H_
#define	_RouteingNumber_H_


#include <asn_application.h>

/* Including external dependencies */
#include "TBCD-STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RouteingNumber */
typedef TBCD_STRING_t	 RouteingNumber_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RouteingNumber;
asn_struct_free_f RouteingNumber_free;
asn_struct_print_f RouteingNumber_print;
asn_constr_check_f RouteingNumber_constraint;
ber_type_decoder_f RouteingNumber_decode_ber;
der_type_encoder_f RouteingNumber_encode_der;
xer_type_decoder_f RouteingNumber_decode_xer;
xer_type_encoder_f RouteingNumber_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _RouteingNumber_H_ */
