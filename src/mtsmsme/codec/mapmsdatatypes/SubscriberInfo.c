#include <asn_internal.h>

#include "SubscriberInfo.h"

static asn_TYPE_member_t asn_MBR_SubscriberInfo_1[] = {
	{ ATF_POINTER, 9, offsetof(struct SubscriberInfo, locationInformation),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"locationInformation"
		},
	{ ATF_POINTER, 8, offsetof(struct SubscriberInfo, subscriberState),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_SubscriberState,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"subscriberState"
		},
	{ ATF_POINTER, 7, offsetof(struct SubscriberInfo, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 6, offsetof(struct SubscriberInfo, locationInformationGPRS),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInformationGPRS,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"locationInformationGPRS"
		},
	{ ATF_POINTER, 5, offsetof(struct SubscriberInfo, ps_SubscriberState),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_PS_SubscriberState,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ps-SubscriberState"
		},
	{ ATF_POINTER, 4, offsetof(struct SubscriberInfo, imei),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMEI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imei"
		},
	{ ATF_POINTER, 3, offsetof(struct SubscriberInfo, ms_Classmark2),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MS_Classmark2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ms-Classmark2"
		},
	{ ATF_POINTER, 2, offsetof(struct SubscriberInfo, gprs_MS_Class),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GPRSMSClass,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gprs-MS-Class"
		},
	{ ATF_POINTER, 1, offsetof(struct SubscriberInfo, mnpInfoRes),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MNPInfoRes,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mnpInfoRes"
		},
};
static ber_tlv_tag_t asn_DEF_SubscriberInfo_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SubscriberInfo_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* locationInformation at 414 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* subscriberState at 415 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* extensionContainer at 416 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* locationInformationGPRS at 418 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* ps-SubscriberState at 419 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* imei at 420 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* ms-Classmark2 at 421 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* gprs-MS-Class at 422 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 } /* mnpInfoRes at 423 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SubscriberInfo_specs_1 = {
	sizeof(struct SubscriberInfo),
	offsetof(struct SubscriberInfo, _asn_ctx),
	asn_MAP_SubscriberInfo_tag2el_1,
	9,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
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
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SubscriberInfo_tags_1,
	sizeof(asn_DEF_SubscriberInfo_tags_1)
		/sizeof(asn_DEF_SubscriberInfo_tags_1[0]), /* 1 */
	asn_DEF_SubscriberInfo_tags_1,	/* Same as above */
	sizeof(asn_DEF_SubscriberInfo_tags_1)
		/sizeof(asn_DEF_SubscriberInfo_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_SubscriberInfo_1,
	9,	/* Elements count */
	&asn_SPC_SubscriberInfo_specs_1	/* Additional specs */
};

