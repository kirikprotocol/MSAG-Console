#ident "$Id$"

#include <asn_internal.h>

#include "AnyTimeSubscriptionInterrogationRes.h"

static asn_TYPE_member_t asn_MBR_AnyTimeSubscriptionInterrogationRes_1[] = {
	{ ATF_POINTER, 9, offsetof(struct AnyTimeSubscriptionInterrogationRes, callForwardingData),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallForwardingData,
		0,	/* Defer constraints checking to the member type */
		"callForwardingData"
		},
	{ ATF_POINTER, 8, offsetof(struct AnyTimeSubscriptionInterrogationRes, callBarringData),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallBarringData,
		0,	/* Defer constraints checking to the member type */
		"callBarringData"
		},
	{ ATF_POINTER, 7, offsetof(struct AnyTimeSubscriptionInterrogationRes, odb_Info),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ODB_Info,
		0,	/* Defer constraints checking to the member type */
		"odb-Info"
		},
	{ ATF_POINTER, 6, offsetof(struct AnyTimeSubscriptionInterrogationRes, camel_SubscriptionInfo),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CAMEL_SubscriptionInfo,
		0,	/* Defer constraints checking to the member type */
		"camel-SubscriptionInfo"
		},
	{ ATF_POINTER, 5, offsetof(struct AnyTimeSubscriptionInterrogationRes, supportedVLR_CAMEL_Phases),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SupportedCamelPhases,
		0,	/* Defer constraints checking to the member type */
		"supportedVLR-CAMEL-Phases"
		},
	{ ATF_POINTER, 4, offsetof(struct AnyTimeSubscriptionInterrogationRes, supportedSGSN_CAMEL_Phases),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SupportedCamelPhases,
		0,	/* Defer constraints checking to the member type */
		"supportedSGSN-CAMEL-Phases"
		},
	{ ATF_POINTER, 3, offsetof(struct AnyTimeSubscriptionInterrogationRes, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 2, offsetof(struct AnyTimeSubscriptionInterrogationRes, offeredCamel4CSIsInVLR),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OfferedCamel4CSIs,
		0,	/* Defer constraints checking to the member type */
		"offeredCamel4CSIsInVLR"
		},
	{ ATF_POINTER, 1, offsetof(struct AnyTimeSubscriptionInterrogationRes, offeredCamel4CSIsInSGSN),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OfferedCamel4CSIs,
		0,	/* Defer constraints checking to the member type */
		"offeredCamel4CSIsInSGSN"
		},
};
static ber_tlv_tag_t asn_DEF_AnyTimeSubscriptionInterrogationRes_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_AnyTimeSubscriptionInterrogationRes_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* callForwardingData at 772 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* callBarringData at 773 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* odb-Info at 774 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* camel-SubscriptionInfo at 775 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 4, 0, 0 }, /* supportedVLR-CAMEL-Phases at 776 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 5, 0, 0 }, /* supportedSGSN-CAMEL-Phases at 777 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 6, 0, 0 }, /* extensionContainer at 778 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 7, 0, 0 }, /* offeredCamel4CSIsInVLR at 780 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 8, 0, 0 } /* offeredCamel4CSIsInSGSN at 781 */
};
static asn_SEQUENCE_specifics_t asn_SPC_AnyTimeSubscriptionInterrogationRes_1_specs = {
	sizeof(struct AnyTimeSubscriptionInterrogationRes),
	offsetof(struct AnyTimeSubscriptionInterrogationRes, _asn_ctx),
	asn_MAP_AnyTimeSubscriptionInterrogationRes_1_tag2el,
	9,	/* Count of tags in the map */
	6,	/* Start extensions */
	10	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_AnyTimeSubscriptionInterrogationRes = {
	"AnyTimeSubscriptionInterrogationRes",
	"AnyTimeSubscriptionInterrogationRes",
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
	asn_DEF_AnyTimeSubscriptionInterrogationRes_1_tags,
	sizeof(asn_DEF_AnyTimeSubscriptionInterrogationRes_1_tags)
		/sizeof(asn_DEF_AnyTimeSubscriptionInterrogationRes_1_tags[0]), /* 1 */
	asn_DEF_AnyTimeSubscriptionInterrogationRes_1_tags,	/* Same as above */
	sizeof(asn_DEF_AnyTimeSubscriptionInterrogationRes_1_tags)
		/sizeof(asn_DEF_AnyTimeSubscriptionInterrogationRes_1_tags[0]), /* 1 */
	asn_MBR_AnyTimeSubscriptionInterrogationRes_1,
	9,	/* Elements count */
	&asn_SPC_AnyTimeSubscriptionInterrogationRes_1_specs	/* Additional specs */
};

