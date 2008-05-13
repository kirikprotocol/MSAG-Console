#ifndef	_SuppressionOfAnnouncement_H_
#define	_SuppressionOfAnnouncement_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SuppressionOfAnnouncement */
typedef NULL_t	 SuppressionOfAnnouncement_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SuppressionOfAnnouncement;
asn_struct_free_f SuppressionOfAnnouncement_free;
asn_struct_print_f SuppressionOfAnnouncement_print;
asn_constr_check_f SuppressionOfAnnouncement_constraint;
ber_type_decoder_f SuppressionOfAnnouncement_decode_ber;
der_type_encoder_f SuppressionOfAnnouncement_encode_der;
xer_type_decoder_f SuppressionOfAnnouncement_decode_xer;
xer_type_encoder_f SuppressionOfAnnouncement_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _SuppressionOfAnnouncement_H_ */
