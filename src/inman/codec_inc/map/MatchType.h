#ifndef	_MatchType_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_MatchType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum MatchType {
	MatchType_inhibiting	= 0,
	MatchType_enabling	= 1
} MatchType_e;

/* MatchType */
typedef long	 MatchType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MatchType;
asn_struct_free_f MatchType_free;
asn_struct_print_f MatchType_print;
asn_constr_check_f MatchType_constraint;
ber_type_decoder_f MatchType_decode_ber;
der_type_encoder_f MatchType_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f MatchType_decode_xer;
xer_type_encoder_f MatchType_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _MatchType_H_ */
