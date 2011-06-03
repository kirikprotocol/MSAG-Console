#ifndef	_UnavailabilityCause_H_
#ifndef __GNUC__
#ident "$Id$"
#endif
#define	_UnavailabilityCause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum UnavailabilityCause {
	UnavailabilityCause_bearerServiceNotProvisioned	= 1,
	UnavailabilityCause_teleserviceNotProvisioned	= 2,
	UnavailabilityCause_absentSubscriber	= 3,
	UnavailabilityCause_busySubscriber	= 4,
	UnavailabilityCause_callBarred	= 5,
	UnavailabilityCause_cug_Reject	= 6
	/*
	 * Enumeration is extensible
	 */
} UnavailabilityCause_e;

/* UnavailabilityCause */
typedef long	 UnavailabilityCause_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UnavailabilityCause;
asn_struct_free_f UnavailabilityCause_free;
asn_struct_print_f UnavailabilityCause_print;
asn_constr_check_f UnavailabilityCause_constraint;
ber_type_decoder_f UnavailabilityCause_decode_ber;
der_type_encoder_f UnavailabilityCause_encode_der;
#ifndef ASN1_XER_NOT_USED
xer_type_decoder_f UnavailabilityCause_decode_xer;
xer_type_encoder_f UnavailabilityCause_encode_xer;
#endif /* ASN1_XER_NOT_USED */

#ifdef __cplusplus
}
#endif

#endif	/* _UnavailabilityCause_H_ */
