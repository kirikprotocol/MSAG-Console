#include <asn_internal.h>

#include "PCS-Extensions.h"

static ber_tlv_tag_t asn_DEF_PCS_Extensions_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SEQUENCE_specifics_t asn_SPC_PCS_Extensions_specs_1 = {
	sizeof(struct PCS_Extensions),
	offsetof(struct PCS_Extensions, _asn_ctx),
	0,	/* No top level tags */
	0,	/* No tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
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
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_PCS_Extensions_tags_1,
	sizeof(asn_DEF_PCS_Extensions_tags_1)
		/sizeof(asn_DEF_PCS_Extensions_tags_1[0]), /* 1 */
	asn_DEF_PCS_Extensions_tags_1,	/* Same as above */
	sizeof(asn_DEF_PCS_Extensions_tags_1)
		/sizeof(asn_DEF_PCS_Extensions_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	0, 0,	/* No members */
	&asn_SPC_PCS_Extensions_specs_1	/* Additional specs */
};

