#include <asn_internal.h>

#include "T-CSI.h"

static asn_TYPE_member_t asn_MBR_T_CSI_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct T_CSI, t_BcsmCamelTDPDataList),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_T_BcsmCamelTDPDataList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"t-BcsmCamelTDPDataList"
		},
	{ ATF_POINTER, 4, offsetof(struct T_CSI, extensionContainer),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 3, offsetof(struct T_CSI, camelCapabilityHandling),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CamelCapabilityHandling,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"camelCapabilityHandling"
		},
	{ ATF_POINTER, 2, offsetof(struct T_CSI, notificationToCSE),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct T_CSI, csi_Active),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"csi-Active"
		},
};
static ber_tlv_tag_t asn_DEF_T_CSI_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_T_CSI_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 1 }, /* t-BcsmCamelTDPDataList at 338 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, -1, 0 }, /* extensionContainer at 339 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 }, /* camelCapabilityHandling at 341 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 }, /* notificationToCSE at 342 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 4, 0, 0 } /* csi-Active at 343 */
};
static asn_SEQUENCE_specifics_t asn_SPC_T_CSI_specs_1 = {
	sizeof(struct T_CSI),
	offsetof(struct T_CSI, _asn_ctx),
	asn_MAP_T_CSI_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_T_CSI = {
	"T-CSI",
	"T-CSI",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_T_CSI_tags_1,
	sizeof(asn_DEF_T_CSI_tags_1)
		/sizeof(asn_DEF_T_CSI_tags_1[0]), /* 1 */
	asn_DEF_T_CSI_tags_1,	/* Same as above */
	sizeof(asn_DEF_T_CSI_tags_1)
		/sizeof(asn_DEF_T_CSI_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_T_CSI_1,
	5,	/* Elements count */
	&asn_SPC_T_CSI_specs_1	/* Additional specs */
};

