#ident "$Id$"

#include <asn_internal.h>

#include "AnyTimeSubscriptionInterrogationArg.h"

static asn_TYPE_member_t asn_MBR_AnyTimeSubscriptionInterrogationArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct AnyTimeSubscriptionInterrogationArg, subscriberIdentity),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_SubscriberIdentity,
		0,	/* Defer constraints checking to the member type */
		"subscriberIdentity"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AnyTimeSubscriptionInterrogationArg, requestedSubscriptionInfo),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RequestedSubscriptionInfo,
		0,	/* Defer constraints checking to the member type */
		"requestedSubscriptionInfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AnyTimeSubscriptionInterrogationArg, gsmSCF_Address),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"gsmSCF-Address"
		},
	{ ATF_POINTER, 2, offsetof(struct AnyTimeSubscriptionInterrogationArg, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct AnyTimeSubscriptionInterrogationArg, longFTN_Supported),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"longFTN-Supported"
		},
};
static ber_tlv_tag_t asn_DEF_AnyTimeSubscriptionInterrogationArg_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_AnyTimeSubscriptionInterrogationArg_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* subscriberIdentity at 763 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* requestedSubscriptionInfo at 764 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* gsmSCF-Address at 765 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* extensionContainer at 766 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* longFTN-Supported at 767 */
};
static asn_SEQUENCE_specifics_t asn_SPC_AnyTimeSubscriptionInterrogationArg_1_specs = {
	sizeof(struct AnyTimeSubscriptionInterrogationArg),
	offsetof(struct AnyTimeSubscriptionInterrogationArg, _asn_ctx),
	asn_MAP_AnyTimeSubscriptionInterrogationArg_1_tag2el,
	5,	/* Count of tags in the map */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_AnyTimeSubscriptionInterrogationArg = {
	"AnyTimeSubscriptionInterrogationArg",
	"AnyTimeSubscriptionInterrogationArg",
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
	asn_DEF_AnyTimeSubscriptionInterrogationArg_1_tags,
	sizeof(asn_DEF_AnyTimeSubscriptionInterrogationArg_1_tags)
		/sizeof(asn_DEF_AnyTimeSubscriptionInterrogationArg_1_tags[0]), /* 1 */
	asn_DEF_AnyTimeSubscriptionInterrogationArg_1_tags,	/* Same as above */
	sizeof(asn_DEF_AnyTimeSubscriptionInterrogationArg_1_tags)
		/sizeof(asn_DEF_AnyTimeSubscriptionInterrogationArg_1_tags[0]), /* 1 */
	asn_MBR_AnyTimeSubscriptionInterrogationArg_1,
	5,	/* Elements count */
	&asn_SPC_AnyTimeSubscriptionInterrogationArg_1_specs	/* Additional specs */
};

