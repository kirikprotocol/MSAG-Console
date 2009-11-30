#include <asn_internal.h>

#include "DestinationNumberCriteria.h"

static asn_TYPE_member_t asn_MBR_DestinationNumberCriteria_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DestinationNumberCriteria, matchType),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MatchType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"matchType"
		},
	{ ATF_POINTER, 2, offsetof(struct DestinationNumberCriteria, destinationNumberList),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DestinationNumberList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"destinationNumberList"
		},
	{ ATF_POINTER, 1, offsetof(struct DestinationNumberCriteria, destinationNumberLengthList),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DestinationNumberLengthList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"destinationNumberLengthList"
		},
};
static ber_tlv_tag_t asn_DEF_DestinationNumberCriteria_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DestinationNumberCriteria_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* matchType at 254 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* destinationNumberList at 255 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* destinationNumberLengthList at 256 */
};
static asn_SEQUENCE_specifics_t asn_SPC_DestinationNumberCriteria_specs_1 = {
	sizeof(struct DestinationNumberCriteria),
	offsetof(struct DestinationNumberCriteria, _asn_ctx),
	asn_MAP_DestinationNumberCriteria_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	4	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DestinationNumberCriteria = {
	"DestinationNumberCriteria",
	"DestinationNumberCriteria",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DestinationNumberCriteria_tags_1,
	sizeof(asn_DEF_DestinationNumberCriteria_tags_1)
		/sizeof(asn_DEF_DestinationNumberCriteria_tags_1[0]), /* 1 */
	asn_DEF_DestinationNumberCriteria_tags_1,	/* Same as above */
	sizeof(asn_DEF_DestinationNumberCriteria_tags_1)
		/sizeof(asn_DEF_DestinationNumberCriteria_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DestinationNumberCriteria_1,
	3,	/* Elements count */
	&asn_SPC_DestinationNumberCriteria_specs_1	/* Additional specs */
};

