#ifndef	_CancellationType_H_
#define	_CancellationType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CancellationType {
	CancellationType_updateProcedure	= 0,
	CancellationType_subscriptionWithdraw	= 1
	/*
	 * Enumeration is extensible
	 */
} e_CancellationType;

/* CancellationType */
typedef long	 CancellationType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CancellationType;
asn_struct_free_f CancellationType_free;
asn_struct_print_f CancellationType_print;
asn_constr_check_f CancellationType_constraint;
ber_type_decoder_f CancellationType_decode_ber;
der_type_encoder_f CancellationType_encode_der;
xer_type_decoder_f CancellationType_decode_xer;
xer_type_encoder_f CancellationType_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _CancellationType_H_ */
