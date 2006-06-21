#ident "$Id$"

#include <asn_internal.h>

#include "SubscriberInfo.h"

static asn_TYPE_member_t asn_MBR_SubscriberInfo_1[] = {
	{ ATF_POINTER, 9, offsetof(struct SubscriberInfo, locationInformation),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInformation,
		0,	/* Defer constraints checking to the member type */
		"locationInformation"
		},
	{ ATF_POINTER, 8, offsetof(struct SubscriberInfo, subscriberState),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_SubscriberState,
		0,	/* Defer constraints checking to the member type */
		"subscriberState"
		},
	{ ATF_POINTER, 7, offsetof(struct SubscriberInfo, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 6, offsetof(struct SubscriberInfo, locationInformationGPRS),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInformationGPRS,
		0,	/* Defer constraints checking to the member type */
		"locationInformationGPRS"
		},
	{ ATF_POINTER, 5, offsetof(struct SubscriberInfo, ps_SubscriberState),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_PS_SubscriberState,
		0,	/* Defer constraints checking to the member type */
		"ps-SubscriberState"
		},
	{ ATF_POINTER, 4, offsetof(struct SubscriberInfo, imei),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMEI,
		0,	/* Defer constraints checking to the member type */
		"imei"
		},
	{ ATF_POINTER, 3, offsetof(struct SubscriberInfo, ms_Classmark2),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MS_Classmark2,
		0,	/* Defer constraints checking to the member type */
		"ms-Classmark2"
		},
	{ ATF_POINTER, 2, offsetof(struct SubscriberInfo, gprs_MS_Class),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GPRSMSClass,
		0,	/* Defer constraints checking to the member type */
		"gprs-MS-Class"
		},
	{ ATF_POINTER, 1, offsetof(struct SubscriberInfo, mnpInfoRes),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MNPInfoRes,
		0,	/* Defer constraints checking to the member type */
		"mnpInfoRes"
		},
};
static ber_tlv_tag_t asn_DEF_SubscriberInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SubscriberInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* locationInformation at 365 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* subscriberState at 366 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* extensionContainer at 367 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* locationInformationGPRS at 369 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* ps-SubscriberState at 370 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* imei at 371 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* ms-Classmark2 at 372 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* gprs-MS-Class at 373 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 } /* mnpInfoRes at 374 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SubscriberInfo_1_specs = {
	sizeof(struct SubscriberInfo),
	offsetof(struct SubscriberInfo, _asn_ctx),
	asn_MAP_SubscriberInfo_1_tag2el,
	9,	/* Count of tags in the map */
	2,	/* Start extensions */
	10	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SubscriberInfo = {
	"SubscriberInfo",
	"SubscriberInfo",
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
	asn_DEF_SubscriberInfo_1_tags,
	sizeof(asn_DEF_SubscriberInfo_1_tags)
		/sizeof(asn_DEF_SubscriberInfo_1_tags[0]), /* 1 */
	asn_DEF_SubscriberInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_SubscriberInfo_1_tags)
		/sizeof(asn_DEF_SubscriberInfo_1_tags[0]), /* 1 */
	asn_MBR_SubscriberInfo_1,
	9,	/* Elements count */
	&asn_SPC_SubscriberInfo_1_specs	/* Additional specs */
};

