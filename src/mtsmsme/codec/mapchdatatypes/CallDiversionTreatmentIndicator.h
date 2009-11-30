#ifndef	_CallDiversionTreatmentIndicator_H_
#define	_CallDiversionTreatmentIndicator_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CallDiversionTreatmentIndicator */
typedef OCTET_STRING_t	 CallDiversionTreatmentIndicator_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CallDiversionTreatmentIndicator;
asn_struct_free_f CallDiversionTreatmentIndicator_free;
asn_struct_print_f CallDiversionTreatmentIndicator_print;
asn_constr_check_f CallDiversionTreatmentIndicator_constraint;
ber_type_decoder_f CallDiversionTreatmentIndicator_decode_ber;
der_type_encoder_f CallDiversionTreatmentIndicator_encode_der;
xer_type_decoder_f CallDiversionTreatmentIndicator_decode_xer;
xer_type_encoder_f CallDiversionTreatmentIndicator_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CallDiversionTreatmentIndicator_H_ */
