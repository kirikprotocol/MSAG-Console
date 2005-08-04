#ifndef	_UnavailableNetworkResource_H_
#define	_UnavailableNetworkResource_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum UnavailableNetworkResource {
	UnavailableNetworkResource_unavailableResources	= 0,
	UnavailableNetworkResource_componentFailure	= 1,
	UnavailableNetworkResource_basicCallProcessingException	= 2,
	UnavailableNetworkResource_resourceStatusFailure	= 3,
	UnavailableNetworkResource_endUserFailure	= 4
} UnavailableNetworkResource_e;

/* UnavailableNetworkResource */
typedef long	 UnavailableNetworkResource_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UnavailableNetworkResource;
asn_struct_free_f UnavailableNetworkResource_free;
asn_struct_print_f UnavailableNetworkResource_print;
asn_constr_check_f UnavailableNetworkResource_constraint;
ber_type_decoder_f UnavailableNetworkResource_decode_ber;
der_type_encoder_f UnavailableNetworkResource_encode_der;
xer_type_decoder_f UnavailableNetworkResource_decode_xer;
xer_type_encoder_f UnavailableNetworkResource_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _UnavailableNetworkResource_H_ */
