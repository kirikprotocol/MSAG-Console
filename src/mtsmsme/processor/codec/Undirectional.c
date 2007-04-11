#include <asn_internal.h>

#include "Undirectional.h"

static asn_TYPE_member_t asn_MBR_Undirectional_1[] = {
	{ ATF_POINTER, 1, offsetof(struct Undirectional, dialoguePortion),
		(ASN_TAG_CLASS_APPLICATION | (11 << 2)),
		0,
		&asn_DEF_DialoguePortion,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dialoguePortion"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Undirectional, components),
		(ASN_TAG_CLASS_APPLICATION | (12 << 2)),
		0,
		&asn_DEF_ComponentPortion,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"components"
		},
};
static ber_tlv_tag_t asn_DEF_Undirectional_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Undirectional_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 0, 0, 0 }, /* dialoguePortion at 28 */
    { (ASN_TAG_CLASS_APPLICATION | (12 << 2)), 1, 0, 0 } /* components at 30 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Undirectional_specs_1 = {
	sizeof(struct Undirectional),
	offsetof(struct Undirectional, _asn_ctx),
	asn_MAP_Undirectional_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Undirectional = {
	"Undirectional",
	"Undirectional",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Undirectional_tags_1,
	sizeof(asn_DEF_Undirectional_tags_1)
		/sizeof(asn_DEF_Undirectional_tags_1[0]), /* 1 */
	asn_DEF_Undirectional_tags_1,	/* Same as above */
	sizeof(asn_DEF_Undirectional_tags_1)
		/sizeof(asn_DEF_Undirectional_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Undirectional_1,
	2,	/* Elements count */
	&asn_SPC_Undirectional_specs_1	/* Additional specs */
};

