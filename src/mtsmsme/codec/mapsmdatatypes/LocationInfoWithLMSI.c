#include <asn_internal.h>

#include "LocationInfoWithLMSI.h"

static asn_TYPE_member_t asn_MBR_LocationInfoWithLMSI_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct LocationInfoWithLMSI, networkNode_Number),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"networkNode-Number"
		},
	{ ATF_POINTER, 4, offsetof(struct LocationInfoWithLMSI, lmsi),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_LMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lmsi"
		},
	{ ATF_POINTER, 3, offsetof(struct LocationInfoWithLMSI, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 2, offsetof(struct LocationInfoWithLMSI, gprsNodeIndicator),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gprsNodeIndicator"
		},
	{ ATF_POINTER, 1, offsetof(struct LocationInfoWithLMSI, additional_Number),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_Additional_Number,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"additional-Number"
		},
};
static ber_tlv_tag_t asn_DEF_LocationInfoWithLMSI_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_LocationInfoWithLMSI_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, 0, 0 }, /* lmsi at 93 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, 0, 0 }, /* extensionContainer at 94 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* networkNode-Number at 92 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 3, 0, 0 }, /* gprsNodeIndicator at 96 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 4, 0, 0 } /* additional-Number at 99 */
};
static asn_SEQUENCE_specifics_t asn_SPC_LocationInfoWithLMSI_specs_1 = {
	sizeof(struct LocationInfoWithLMSI),
	offsetof(struct LocationInfoWithLMSI, _asn_ctx),
	asn_MAP_LocationInfoWithLMSI_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	2,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_LocationInfoWithLMSI = {
	"LocationInfoWithLMSI",
	"LocationInfoWithLMSI",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_LocationInfoWithLMSI_tags_1,
	sizeof(asn_DEF_LocationInfoWithLMSI_tags_1)
		/sizeof(asn_DEF_LocationInfoWithLMSI_tags_1[0]), /* 1 */
	asn_DEF_LocationInfoWithLMSI_tags_1,	/* Same as above */
	sizeof(asn_DEF_LocationInfoWithLMSI_tags_1)
		/sizeof(asn_DEF_LocationInfoWithLMSI_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_LocationInfoWithLMSI_1,
	5,	/* Elements count */
	&asn_SPC_LocationInfoWithLMSI_specs_1	/* Additional specs */
};

