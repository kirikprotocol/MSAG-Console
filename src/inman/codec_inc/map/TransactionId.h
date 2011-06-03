#ifndef	_TransactionId_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_TransactionId_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TransactionId */
typedef OCTET_STRING_t	 TransactionId_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TransactionId;
asn_struct_free_f TransactionId_free;
asn_struct_print_f TransactionId_print;
asn_constr_check_f TransactionId_constraint;
ber_type_decoder_f TransactionId_decode_ber;
der_type_encoder_f TransactionId_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f TransactionId_decode_xer;
xer_type_encoder_f TransactionId_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _TransactionId_H_ */
