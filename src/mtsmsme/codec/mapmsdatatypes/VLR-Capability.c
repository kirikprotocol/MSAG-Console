#include <asn_internal.h>

#include "VLR-Capability.h"

static asn_TYPE_member_t asn_MBR_VLR_Capability_1[] = {
	{ ATF_POINTER, 8, offsetof(struct VLR_Capability, supportedCamelPhases),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SupportedCamelPhases,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"supportedCamelPhases"
		},
	{ ATF_POINTER, 7, offsetof(struct VLR_Capability, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 6, offsetof(struct VLR_Capability, solsaSupportIndicator),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"solsaSupportIndicator"
		},
	{ ATF_POINTER, 5, offsetof(struct VLR_Capability, istSupportIndicator),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IST_SupportIndicator,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"istSupportIndicator"
		},
	{ ATF_POINTER, 4, offsetof(struct VLR_Capability, superChargerSupportedInServingNetworkEntity),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_SuperChargerInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"superChargerSupportedInServingNetworkEntity"
		},
	{ ATF_POINTER, 3, offsetof(struct VLR_Capability, longFTN_Supported),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"longFTN-Supported"
		},
	{ ATF_POINTER, 2, offsetof(struct VLR_Capability, supportedLCS_CapabilitySets),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SupportedLCS_CapabilitySets,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"supportedLCS-CapabilitySets"
		},
	{ ATF_POINTER, 1, offsetof(struct VLR_Capability, offeredCamel4CSIs),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OfferedCamel4CSIs,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"offeredCamel4CSIs"
		},
};
static ber_tlv_tag_t asn_DEF_VLR_Capability_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_VLR_Capability_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 }, /* extensionContainer at 85 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* supportedCamelPhases at 84 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 }, /* istSupportIndicator at 88 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* solsaSupportIndicator at 87 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 4, 0, 0 }, /* superChargerSupportedInServingNetworkEntity at 89 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 5, 0, 0 }, /* longFTN-Supported at 90 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 6, 0, 0 }, /* supportedLCS-CapabilitySets at 91 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 7, 0, 0 } /* offeredCamel4CSIs at 92 */
};
static asn_SEQUENCE_specifics_t asn_SPC_VLR_Capability_specs_1 = {
	sizeof(struct VLR_Capability),
	offsetof(struct VLR_Capability, _asn_ctx),
	asn_MAP_VLR_Capability_tag2el_1,
	8,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	9	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_VLR_Capability = {
	"VLR-Capability",
	"VLR-Capability",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_VLR_Capability_tags_1,
	sizeof(asn_DEF_VLR_Capability_tags_1)
		/sizeof(asn_DEF_VLR_Capability_tags_1[0]), /* 1 */
	asn_DEF_VLR_Capability_tags_1,	/* Same as above */
	sizeof(asn_DEF_VLR_Capability_tags_1)
		/sizeof(asn_DEF_VLR_Capability_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_VLR_Capability_1,
	8,	/* Elements count */
	&asn_SPC_VLR_Capability_specs_1	/* Additional specs */
};

