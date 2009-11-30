#include <asn_internal.h>

#include "D-CSI.h"

static asn_TYPE_member_t asn_MBR_D_CSI_1[] = {
	{ ATF_POINTER, 5, offsetof(struct D_CSI, dp_AnalysedInfoCriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DP_AnalysedInfoCriteriaList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dp-AnalysedInfoCriteriaList"
		},
	{ ATF_POINTER, 4, offsetof(struct D_CSI, camelCapabilityHandling),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CamelCapabilityHandling,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"camelCapabilityHandling"
		},
	{ ATF_POINTER, 3, offsetof(struct D_CSI, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 2, offsetof(struct D_CSI, notificationToCSE),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct D_CSI, csi_Active),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"csi-Active"
		},
};
static ber_tlv_tag_t asn_DEF_D_CSI_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_D_CSI_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* dp-AnalysedInfoCriteriaList at 162 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* camelCapabilityHandling at 163 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* extensionContainer at 164 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* notificationToCSE at 165 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* csi-Active at 166 */
};
static asn_SEQUENCE_specifics_t asn_SPC_D_CSI_specs_1 = {
	sizeof(struct D_CSI),
	offsetof(struct D_CSI, _asn_ctx),
	asn_MAP_D_CSI_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_D_CSI = {
	"D-CSI",
	"D-CSI",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_D_CSI_tags_1,
	sizeof(asn_DEF_D_CSI_tags_1)
		/sizeof(asn_DEF_D_CSI_tags_1[0]), /* 1 */
	asn_DEF_D_CSI_tags_1,	/* Same as above */
	sizeof(asn_DEF_D_CSI_tags_1)
		/sizeof(asn_DEF_D_CSI_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_D_CSI_1,
	5,	/* Elements count */
	&asn_SPC_D_CSI_specs_1	/* Additional specs */
};

