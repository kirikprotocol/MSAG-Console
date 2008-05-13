#include <asn_internal.h>

#include "Begin.h"

static asn_TYPE_member_t asn_MBR_Begin_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Begin, otid),
		(ASN_TAG_CLASS_APPLICATION | (8 << 2)),
		0,
		&asn_DEF_OrigTransactionID,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"otid"
		},
	{ ATF_POINTER, 2, offsetof(struct Begin, dialoguePortion),
		(ASN_TAG_CLASS_APPLICATION | (11 << 2)),
		0,
		&asn_DEF_DialoguePortion,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dialoguePortion"
		},
	{ ATF_POINTER, 1, offsetof(struct Begin, components),
		(ASN_TAG_CLASS_APPLICATION | (12 << 2)),
		0,
		&asn_DEF_ComponentPortion,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"components"
		},
};
static ber_tlv_tag_t asn_DEF_Begin_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Begin_tag2el_1[] = {
    { (ASN_TAG_CLASS_APPLICATION | (8 << 2)), 0, 0, 0 }, /* otid at 34 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 1, 0, 0 }, /* dialoguePortion at 35 */
    { (ASN_TAG_CLASS_APPLICATION | (12 << 2)), 2, 0, 0 } /* components at 36 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Begin_specs_1 = {
	sizeof(struct Begin),
	offsetof(struct Begin, _asn_ctx),
	asn_MAP_Begin_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Begin = {
	"Begin",
	"Begin",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Begin_tags_1,
	sizeof(asn_DEF_Begin_tags_1)
		/sizeof(asn_DEF_Begin_tags_1[0]), /* 1 */
	asn_DEF_Begin_tags_1,	/* Same as above */
	sizeof(asn_DEF_Begin_tags_1)
		/sizeof(asn_DEF_Begin_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Begin_1,
	3,	/* Elements count */
	&asn_SPC_Begin_specs_1	/* Additional specs */
};

