#ifndef	_ForwardingReason_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_ForwardingReason_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ForwardingReason {
	ForwardingReason_notReachable	= 0,
	ForwardingReason_busy	= 1,
	ForwardingReason_noReply	= 2
} ForwardingReason_e;

/* ForwardingReason */
typedef long	 ForwardingReason_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ForwardingReason;
asn_struct_free_f ForwardingReason_free;
asn_struct_print_f ForwardingReason_print;
asn_constr_check_f ForwardingReason_constraint;
ber_type_decoder_f ForwardingReason_decode_ber;
der_type_encoder_f ForwardingReason_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f ForwardingReason_decode_xer;
xer_type_encoder_f ForwardingReason_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _ForwardingReason_H_ */
