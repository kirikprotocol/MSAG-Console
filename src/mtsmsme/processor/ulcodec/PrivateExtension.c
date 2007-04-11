#include <asn_internal.h>

#include "PrivateExtension.h"

static asn_TYPE_member_t asn_MBR_PrivateExtension_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PrivateExtension, extId),
		(ASN_TAG_CLASS_UNIVERSAL | (6 << 2)),
		0,
		&asn_DEF_OBJECT_IDENTIFIER,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extId"
		},
	{ ATF_OPEN_TYPE | ATF_NOFLAGS, 0, offsetof(struct PrivateExtension, extType),
		-1 /* Ambiguous tag (ANY?) */,
		0,
		&asn_DEF_ANY,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extType"
		},
};
static ber_tlv_tag_t asn_DEF_PrivateExtension_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_PrivateExtension_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (6 << 2)), 0, 0, 0 } /* extId at 42 */
};
static asn_SEQUENCE_specifics_t asn_SPC_PrivateExtension_specs_1 = {
	sizeof(struct PrivateExtension),
	offsetof(struct PrivateExtension, _asn_ctx),
	asn_MAP_PrivateExtension_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
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
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_PrivateExtension_tags_1,
	sizeof(asn_DEF_PrivateExtension_tags_1)
		/sizeof(asn_DEF_PrivateExtension_tags_1[0]), /* 1 */
	asn_DEF_PrivateExtension_tags_1,	/* Same as above */
	sizeof(asn_DEF_PrivateExtension_tags_1)
		/sizeof(asn_DEF_PrivateExtension_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_PrivateExtension_1,
	2,	/* Elements count */
	&asn_SPC_PrivateExtension_specs_1	/* Additional specs */
};

