#ident "$Id$"

#include <asn_internal.h>

#include "PDP-ContextInfo.h"

static asn_TYPE_member_t asn_MBR_PDP_ContextInfo_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PDP_ContextInfo, pdp_ContextIdentifier),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ContextId,
		0,	/* Defer constraints checking to the member type */
		"pdp-ContextIdentifier"
		},
	{ ATF_POINTER, 1, offsetof(struct PDP_ContextInfo, pdp_ContextActive),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"pdp-ContextActive"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PDP_ContextInfo, pdp_Type),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PDP_Type,
		0,	/* Defer constraints checking to the member type */
		"pdp-Type"
		},
	{ ATF_POINTER, 18, offsetof(struct PDP_ContextInfo, pdp_Address),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PDP_Address,
		0,	/* Defer constraints checking to the member type */
		"pdp-Address"
		},
	{ ATF_POINTER, 17, offsetof(struct PDP_ContextInfo, apn_Subscribed),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_APN,
		0,	/* Defer constraints checking to the member type */
		"apn-Subscribed"
		},
	{ ATF_POINTER, 16, offsetof(struct PDP_ContextInfo, apn_InUse),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_APN,
		0,	/* Defer constraints checking to the member type */
		"apn-InUse"
		},
	{ ATF_POINTER, 15, offsetof(struct PDP_ContextInfo, nsapi),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NSAPI,
		0,	/* Defer constraints checking to the member type */
		"nsapi"
		},
	{ ATF_POINTER, 14, offsetof(struct PDP_ContextInfo, transactionId),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TransactionId,
		0,	/* Defer constraints checking to the member type */
		"transactionId"
		},
	{ ATF_POINTER, 13, offsetof(struct PDP_ContextInfo, teid_ForGnAndGp),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TEID,
		0,	/* Defer constraints checking to the member type */
		"teid-ForGnAndGp"
		},
	{ ATF_POINTER, 12, offsetof(struct PDP_ContextInfo, teid_ForIu),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TEID,
		0,	/* Defer constraints checking to the member type */
		"teid-ForIu"
		},
	{ ATF_POINTER, 11, offsetof(struct PDP_ContextInfo, ggsn_Address),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GSN_Address,
		0,	/* Defer constraints checking to the member type */
		"ggsn-Address"
		},
	{ ATF_POINTER, 10, offsetof(struct PDP_ContextInfo, qos_Subscribed),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_QoS_Subscribed,
		0,	/* Defer constraints checking to the member type */
		"qos-Subscribed"
		},
	{ ATF_POINTER, 9, offsetof(struct PDP_ContextInfo, qos_Requested),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_QoS_Subscribed,
		0,	/* Defer constraints checking to the member type */
		"qos-Requested"
		},
	{ ATF_POINTER, 8, offsetof(struct PDP_ContextInfo, qos_Negotiated),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_QoS_Subscribed,
		0,	/* Defer constraints checking to the member type */
		"qos-Negotiated"
		},
	{ ATF_POINTER, 7, offsetof(struct PDP_ContextInfo, chargingId),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GPRSChargingID,
		0,	/* Defer constraints checking to the member type */
		"chargingId"
		},
	{ ATF_POINTER, 6, offsetof(struct PDP_ContextInfo, chargingCharacteristics),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ChargingCharacteristics,
		0,	/* Defer constraints checking to the member type */
		"chargingCharacteristics"
		},
	{ ATF_POINTER, 5, offsetof(struct PDP_ContextInfo, rnc_Address),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GSN_Address,
		0,	/* Defer constraints checking to the member type */
		"rnc-Address"
		},
	{ ATF_POINTER, 4, offsetof(struct PDP_ContextInfo, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 3, offsetof(struct PDP_ContextInfo, qos2_Subscribed),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext2_QoS_Subscribed,
		0,	/* Defer constraints checking to the member type */
		"qos2-Subscribed"
		},
	{ ATF_POINTER, 2, offsetof(struct PDP_ContextInfo, qos2_Requested),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext2_QoS_Subscribed,
		0,	/* Defer constraints checking to the member type */
		"qos2-Requested"
		},
	{ ATF_POINTER, 1, offsetof(struct PDP_ContextInfo, qos2_Negotiated),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext2_QoS_Subscribed,
		0,	/* Defer constraints checking to the member type */
		"qos2-Negotiated"
		},
};
static ber_tlv_tag_t asn_DEF_PDP_ContextInfo_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_PDP_ContextInfo_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* pdp-ContextIdentifier at 523 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* pdp-ContextActive at 524 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* pdp-Type at 525 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* pdp-Address at 526 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* apn-Subscribed at 527 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* apn-InUse at 528 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* nsapi at 529 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* transactionId at 530 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* teid-ForGnAndGp at 531 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* teid-ForIu at 532 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* ggsn-Address at 533 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* qos-Subscribed at 534 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* qos-Requested at 535 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* qos-Negotiated at 536 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* chargingId at 537 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* chargingCharacteristics at 538 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* rnc-Address at 539 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* extensionContainer at 540 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* qos2-Subscribed at 542 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* qos2-Requested at 544 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 } /* qos2-Negotiated at 546 */
};
static asn_SEQUENCE_specifics_t asn_SPC_PDP_ContextInfo_1_specs = {
	sizeof(struct PDP_ContextInfo),
	offsetof(struct PDP_ContextInfo, _asn_ctx),
	asn_MAP_PDP_ContextInfo_1_tag2el,
	21,	/* Count of tags in the map */
	17,	/* Start extensions */
	22	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_PDP_ContextInfo = {
	"PDP-ContextInfo",
	"PDP-ContextInfo",
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
	asn_DEF_PDP_ContextInfo_1_tags,
	sizeof(asn_DEF_PDP_ContextInfo_1_tags)
		/sizeof(asn_DEF_PDP_ContextInfo_1_tags[0]), /* 1 */
	asn_DEF_PDP_ContextInfo_1_tags,	/* Same as above */
	sizeof(asn_DEF_PDP_ContextInfo_1_tags)
		/sizeof(asn_DEF_PDP_ContextInfo_1_tags[0]), /* 1 */
	asn_MBR_PDP_ContextInfo_1,
	21,	/* Elements count */
	&asn_SPC_PDP_ContextInfo_1_specs	/* Additional specs */
};

