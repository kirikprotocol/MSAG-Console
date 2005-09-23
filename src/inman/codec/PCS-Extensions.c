#include <asn_internal.h>

#include "PCS-Extensions.h"

static ber_tlv_tag_t asn_DEF_PCS_Extensions_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SEQUENCE_specifics_t asn_SPC_PCS_Extensions_1_specs = {
	sizeof(struct PCS_Extensions),
	offsetof(struct PCS_Extensions, _asn_ctx),
	0,	/* No top level tags */
	0,	/* No tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_PCS_Extensions = {
	"PCS-Extensions",
	"PCS-Extensions",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
#ifndef ASN1_XER_NOT_USED
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_PCS_Extensions_1_tags,
	sizeof(asn_DEF_PCS_Extensions_1_tags)
		/sizeof(asn_DEF_PCS_Extensions_1_tags[0]), /* 1 */
	asn_DEF_PCS_Extensions_1_tags,	/* Same as above */
	sizeof(asn_DEF_PCS_Extensions_1_tags)
		/sizeof(asn_DEF_PCS_Extensions_1_tags[0]), /* 1 */
	0, 0,	/* No members */
	&asn_SPC_PCS_Extensions_1_specs	/* Additional specs */
};

