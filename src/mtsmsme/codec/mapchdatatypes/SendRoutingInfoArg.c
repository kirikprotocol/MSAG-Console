#include <asn_internal.h>

#include "SendRoutingInfoArg.h"

static asn_TYPE_member_t asn_MBR_SendRoutingInfoArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SendRoutingInfoArg, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_POINTER, 2, offsetof(struct SendRoutingInfoArg, cug_CheckInfo),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CUG_CheckInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cug-CheckInfo"
		},
	{ ATF_POINTER, 1, offsetof(struct SendRoutingInfoArg, numberOfForwarding),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NumberOfForwarding,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"numberOfForwarding"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SendRoutingInfoArg, interrogationType),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_InterrogationType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"interrogationType"
		},
	{ ATF_POINTER, 2, offsetof(struct SendRoutingInfoArg, or_Interrogation),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"or-Interrogation"
		},
	{ ATF_POINTER, 1, offsetof(struct SendRoutingInfoArg, or_Capability),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OR_Phase,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"or-Capability"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SendRoutingInfoArg, gmsc_OrGsmSCF_Address),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gmsc-OrGsmSCF-Address"
		},
	{ ATF_POINTER, 20, offsetof(struct SendRoutingInfoArg, callReferenceNumber),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallReferenceNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"callReferenceNumber"
		},
	{ ATF_POINTER, 19, offsetof(struct SendRoutingInfoArg, forwardingReason),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ForwardingReason,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"forwardingReason"
		},
	{ ATF_POINTER, 18, offsetof(struct SendRoutingInfoArg, basicServiceGroup),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_Ext_BasicServiceCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"basicServiceGroup"
		},
	{ ATF_POINTER, 17, offsetof(struct SendRoutingInfoArg, networkSignalInfo),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"networkSignalInfo"
		},
	{ ATF_POINTER, 16, offsetof(struct SendRoutingInfoArg, camelInfo),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CamelInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"camelInfo"
		},
	{ ATF_POINTER, 15, offsetof(struct SendRoutingInfoArg, suppressionOfAnnouncement),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SuppressionOfAnnouncement,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"suppressionOfAnnouncement"
		},
	{ ATF_POINTER, 14, offsetof(struct SendRoutingInfoArg, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 13, offsetof(struct SendRoutingInfoArg, alertingPattern),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AlertingPattern,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"alertingPattern"
		},
	{ ATF_POINTER, 12, offsetof(struct SendRoutingInfoArg, ccbs_Call),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ccbs-Call"
		},
	{ ATF_POINTER, 11, offsetof(struct SendRoutingInfoArg, supportedCCBS_Phase),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SupportedCCBS_Phase,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"supportedCCBS-Phase"
		},
	{ ATF_POINTER, 10, offsetof(struct SendRoutingInfoArg, additionalSignalInfo),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"additionalSignalInfo"
		},
	{ ATF_POINTER, 9, offsetof(struct SendRoutingInfoArg, istSupportIndicator),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IST_SupportIndicator,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"istSupportIndicator"
		},
	{ ATF_POINTER, 8, offsetof(struct SendRoutingInfoArg, pre_pagingSupported),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pre-pagingSupported"
		},
	{ ATF_POINTER, 7, offsetof(struct SendRoutingInfoArg, callDiversionTreatmentIndicator),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallDiversionTreatmentIndicator,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"callDiversionTreatmentIndicator"
		},
	{ ATF_POINTER, 6, offsetof(struct SendRoutingInfoArg, longFTN_Supported),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"longFTN-Supported"
		},
	{ ATF_POINTER, 5, offsetof(struct SendRoutingInfoArg, suppress_VT_CSI),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"suppress-VT-CSI"
		},
	{ ATF_POINTER, 4, offsetof(struct SendRoutingInfoArg, suppressIncomingCallBarring),
		(ASN_TAG_CLASS_CONTEXT | (23 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"suppressIncomingCallBarring"
		},
	{ ATF_POINTER, 3, offsetof(struct SendRoutingInfoArg, gsmSCF_InitiatedCall),
		(ASN_TAG_CLASS_CONTEXT | (24 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gsmSCF-InitiatedCall"
		},
	{ ATF_POINTER, 2, offsetof(struct SendRoutingInfoArg, basicServiceGroup2),
		(ASN_TAG_CLASS_CONTEXT | (25 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_Ext_BasicServiceCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"basicServiceGroup2"
		},
	{ ATF_POINTER, 1, offsetof(struct SendRoutingInfoArg, networkSignalInfo2),
		(ASN_TAG_CLASS_CONTEXT | (26 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"networkSignalInfo2"
		},
};
static ber_tlv_tag_t asn_DEF_SendRoutingInfoArg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SendRoutingInfoArg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* msisdn at 74 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* cug-CheckInfo at 75 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* numberOfForwarding at 76 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* interrogationType at 77 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* or-Interrogation at 78 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* or-Capability at 79 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* gmsc-OrGsmSCF-Address at 80 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* callReferenceNumber at 81 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* forwardingReason at 82 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* basicServiceGroup at 83 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* networkSignalInfo at 84 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* camelInfo at 85 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* suppressionOfAnnouncement at 86 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* extensionContainer at 87 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* alertingPattern at 89 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* ccbs-Call at 90 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* supportedCCBS-Phase at 91 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* additionalSignalInfo at 92 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* istSupportIndicator at 93 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* pre-pagingSupported at 94 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* callDiversionTreatmentIndicator at 95 */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* longFTN-Supported at 96 */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 }, /* suppress-VT-CSI at 97 */
    { (ASN_TAG_CLASS_CONTEXT | (23 << 2)), 23, 0, 0 }, /* suppressIncomingCallBarring at 98 */
    { (ASN_TAG_CLASS_CONTEXT | (24 << 2)), 24, 0, 0 }, /* gsmSCF-InitiatedCall at 99 */
    { (ASN_TAG_CLASS_CONTEXT | (25 << 2)), 25, 0, 0 }, /* basicServiceGroup2 at 100 */
    { (ASN_TAG_CLASS_CONTEXT | (26 << 2)), 26, 0, 0 } /* networkSignalInfo2 at 101 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SendRoutingInfoArg_specs_1 = {
	sizeof(struct SendRoutingInfoArg),
	offsetof(struct SendRoutingInfoArg, _asn_ctx),
	asn_MAP_SendRoutingInfoArg_tag2el_1,
	27,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	13,	/* Start extensions */
	28	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoArg = {
	"SendRoutingInfoArg",
	"SendRoutingInfoArg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SendRoutingInfoArg_tags_1,
	sizeof(asn_DEF_SendRoutingInfoArg_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoArg_tags_1[0]), /* 1 */
	asn_DEF_SendRoutingInfoArg_tags_1,	/* Same as above */
	sizeof(asn_DEF_SendRoutingInfoArg_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoArg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_SendRoutingInfoArg_1,
	27,	/* Elements count */
	&asn_SPC_SendRoutingInfoArg_specs_1	/* Additional specs */
};

