#ident "$Id$"

#include <asn_internal.h>

#include "GmscCamelSubscriptionInfo.h"

static asn_TYPE_member_t asn_MBR_GmscCamelSubscriptionInfo_1[] = {
	{ ATF_POINTER, 6, offsetof(struct GmscCamelSubscriptionInfo, t_CSI),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_CSI,
		0,	/* Defer constraints checking to the member type */
		"t-CSI"
		},
	{ ATF_POINTER, 5, offsetof(struct GmscCamelSubscriptionInfo, o_CSI),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_O_CSI,
		0,	/* Defer constraints checking to the member type */
		"o-CSI"
		},
	{ ATF_POINTER, 4, offsetof(struct GmscCamelSubscriptionInfo, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 3, offsetof(struct GmscCamelSubscriptionInfo, o_BcsmCamelTDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_O_BcsmCamelTDPCriteriaList,
		0,	/* Defer constraints checking to the member type */
		"o-BcsmCamelTDP-CriteriaList"
		},
	{ ATF_POINTER, 2, offsetof(struct GmscCamelSubscriptionInfo, t_BCSM_CAMEL_TDP_CriteriaList),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_T_BCSM_CAMEL_TDP_CriteriaList,
		0,	/* Defer constraints checking to the member type */
		"t-BCSM-CAMEL-TDP-CriteriaList"
		},
	{ ATF_POINTER, 1, offsetof(struct GmscCamelSubscriptionInfo, d_csi),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_D_CSI,
		0,	/* Defer constraints checking to the member type */
		"d-csi"
		},
};
static ber_tlv_tag_t asn_DEF_GmscCamelSubscriptionInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_GmscCamelSubscriptionInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* t-CSI at 225 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* o-CSI at 226 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* extensionContainer at 227 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* o-BcsmCamelTDP-CriteriaList at 229 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* t-BCSM-CAMEL-TDP-CriteriaList at 230 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* d-csi at 231 */
};
static asn_SEQUENCE_specifics_t asn_SPC_GmscCamelSubscriptionInfo_1_specs = {
	sizeof(struct GmscCamelSubscriptionInfo),
	offsetof(struct GmscCamelSubscriptionInfo, _asn_ctx),
	asn_MAP_GmscCamelSubscriptionInfo_1_tag2el,
	6,	/* Count of tags in the map */
	2,	/* Start extensions */
	7	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_GmscCamelSubscriptionInfo = {
	"GmscCamelSubscriptionInfo",
	"GmscCamelSubscriptionInfo",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
#ifndef ASN1_XER_NOT_USED
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_GmscCamelSubscriptionInfo_1_tags,
	sizeof(asn_DEF_GmscCamelSubscriptionInfo_1_tags)
		/sizeof(asn_DEF_GmscCamelSubscriptionInfo_1_tags[0]), /* 1 */
	asn_DEF_GmscCamelSubscriptionInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_GmscCamelSubscriptionInfo_1_tags)
		/sizeof(asn_DEF_GmscCamelSubscriptionInfo_1_tags[0]), /* 1 */
	asn_MBR_GmscCamelSubscriptionInfo_1,
	6,	/* Elements count */
	&asn_SPC_GmscCamelSubscriptionInfo_1_specs	/* Additional specs */
};

