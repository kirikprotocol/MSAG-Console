#ident "$Id$"

#include <asn_internal.h>

#include "RequestedSubscriptionInfo.h"

static asn_TYPE_member_t asn_MBR_RequestedSubscriptionInfo_1[] = {
	{ ATF_POINTER, 7, offsetof(struct RequestedSubscriptionInfo, requestedSS_Info),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SS_ForBS_Code,
		0,	/* Defer constraints checking to the member type */
		"requestedSS-Info"
		},
	{ ATF_POINTER, 6, offsetof(struct RequestedSubscriptionInfo, odb),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"odb"
		},
	{ ATF_POINTER, 5, offsetof(struct RequestedSubscriptionInfo, requestedCAMEL_SubscriptionInfo),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RequestedCAMEL_SubscriptionInfo,
		0,	/* Defer constraints checking to the member type */
		"requestedCAMEL-SubscriptionInfo"
		},
	{ ATF_POINTER, 4, offsetof(struct RequestedSubscriptionInfo, supportedVLR_CAMEL_Phases),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"supportedVLR-CAMEL-Phases"
		},
	{ ATF_POINTER, 3, offsetof(struct RequestedSubscriptionInfo, supportedSGSN_CAMEL_Phases),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"supportedSGSN-CAMEL-Phases"
		},
	{ ATF_POINTER, 2, offsetof(struct RequestedSubscriptionInfo, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct RequestedSubscriptionInfo, additionalRequestedCAMEL_SubscriptionInfo),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AdditionalRequestedCAMEL_SubscriptionInfo,
		0,	/* Defer constraints checking to the member type */
		"additionalRequestedCAMEL-SubscriptionInfo"
		},
};
static ber_tlv_tag_t asn_DEF_RequestedSubscriptionInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_RequestedSubscriptionInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* requestedSS-Info at 785 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* odb at 786 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* requestedCAMEL-SubscriptionInfo at 788 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* supportedVLR-CAMEL-Phases at 789 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 4, 0, 0 }, /* supportedSGSN-CAMEL-Phases at 790 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 5, 0, 0 }, /* extensionContainer at 791 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 6, 0, 0 } /* additionalRequestedCAMEL-SubscriptionInfo at 795 */
};
static asn_SEQUENCE_specifics_t asn_SPC_RequestedSubscriptionInfo_1_specs = {
	sizeof(struct RequestedSubscriptionInfo),
	offsetof(struct RequestedSubscriptionInfo, _asn_ctx),
	asn_MAP_RequestedSubscriptionInfo_1_tag2el,
	7,	/* Count of tags in the map */
	5,	/* Start extensions */
	8	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_RequestedSubscriptionInfo = {
	"RequestedSubscriptionInfo",
	"RequestedSubscriptionInfo",
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
	asn_DEF_RequestedSubscriptionInfo_1_tags,
	sizeof(asn_DEF_RequestedSubscriptionInfo_1_tags)
		/sizeof(asn_DEF_RequestedSubscriptionInfo_1_tags[0]), /* 1 */
	asn_DEF_RequestedSubscriptionInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_RequestedSubscriptionInfo_1_tags)
		/sizeof(asn_DEF_RequestedSubscriptionInfo_1_tags[0]), /* 1 */
	asn_MBR_RequestedSubscriptionInfo_1,
	7,	/* Elements count */
	&asn_SPC_RequestedSubscriptionInfo_1_specs	/* Additional specs */
};

