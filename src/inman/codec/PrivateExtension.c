#include <asn_internal.h>

#include "PrivateExtension.h"

static asn_TYPE_member_t asn_MBR_PrivateExtension_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PrivateExtension, extId),
		(ASN_TAG_CLASS_UNIVERSAL | (6 << 2)),
		0,
		(void *)&asn_DEF_OBJECT_IDENTIFIER,
		0,	/* Defer constraints checking to the member type */
		"extId"
		},
};
static ber_tlv_tag_t asn_DEF_PrivateExtension_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_PrivateExtension_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 0, 0, 0 } /* extId at 431 */
};
static asn_SEQUENCE_specifics_t asn_SPC_PrivateExtension_1_specs = {
	sizeof(struct PrivateExtension),
	offsetof(struct PrivateExtension, _asn_ctx),
	asn_MAP_PrivateExtension_1_tag2el,
	1,	/* Count of tags in the map */
	0,	/* Start extensions */
	2	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_PrivateExtension = {
	"PrivateExtension",
	"PrivateExtension",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_PrivateExtension_1_tags,
	sizeof(asn_DEF_PrivateExtension_1_tags)
		/sizeof(asn_DEF_PrivateExtension_1_tags[0]), /* 1 */
	asn_DEF_PrivateExtension_1_tags,	/* Same as above */
	sizeof(asn_DEF_PrivateExtension_1_tags)
		/sizeof(asn_DEF_PrivateExtension_1_tags[0]), /* 1 */
	asn_MBR_PrivateExtension_1,
	1,	/* Elements count */
	&asn_SPC_PrivateExtension_1_specs	/* Additional specs */
};

