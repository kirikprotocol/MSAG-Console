#include <asn_internal.h>

#include "ProvideRoamingNumberArg.h"

static asn_TYPE_member_t asn_MBR_ProvideRoamingNumberArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ProvideRoamingNumberArg, imsi),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ProvideRoamingNumberArg, msc_Number),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msc-Number"
		},
	{ ATF_POINTER, 18, offsetof(struct ProvideRoamingNumberArg, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_POINTER, 17, offsetof(struct ProvideRoamingNumberArg, lmsi),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lmsi"
		},
	{ ATF_POINTER, 16, offsetof(struct ProvideRoamingNumberArg, gsm_BearerCapability),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gsm-BearerCapability"
		},
	{ ATF_POINTER, 15, offsetof(struct ProvideRoamingNumberArg, networkSignalInfo),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"networkSignalInfo"
		},
	{ ATF_POINTER, 14, offsetof(struct ProvideRoamingNumberArg, suppressionOfAnnouncement),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SuppressionOfAnnouncement,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"suppressionOfAnnouncement"
		},
	{ ATF_POINTER, 13, offsetof(struct ProvideRoamingNumberArg, gmsc_Address),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gmsc-Address"
		},
	{ ATF_POINTER, 12, offsetof(struct ProvideRoamingNumberArg, callReferenceNumber),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallReferenceNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"callReferenceNumber"
		},
	{ ATF_POINTER, 11, offsetof(struct ProvideRoamingNumberArg, or_Interrogation),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"or-Interrogation"
		},
	{ ATF_POINTER, 10, offsetof(struct ProvideRoamingNumberArg, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 9, offsetof(struct ProvideRoamingNumberArg, alertingPattern),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AlertingPattern,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"alertingPattern"
		},
	{ ATF_POINTER, 8, offsetof(struct ProvideRoamingNumberArg, ccbs_Call),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ccbs-Call"
		},
	{ ATF_POINTER, 7, offsetof(struct ProvideRoamingNumberArg, supportedCamelPhasesInInterrogatingNode),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SupportedCamelPhases,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"supportedCamelPhasesInInterrogatingNode"
		},
	{ ATF_POINTER, 6, offsetof(struct ProvideRoamingNumberArg, additionalSignalInfo),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"additionalSignalInfo"
		},
	{ ATF_POINTER, 5, offsetof(struct ProvideRoamingNumberArg, orNotSupportedInGMSC),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"orNotSupportedInGMSC"
		},
	{ ATF_POINTER, 4, offsetof(struct ProvideRoamingNumberArg, pre_pagingSupported),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pre-pagingSupported"
		},
	{ ATF_POINTER, 3, offsetof(struct ProvideRoamingNumberArg, longFTN_Supported),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"longFTN-Supported"
		},
	{ ATF_POINTER, 2, offsetof(struct ProvideRoamingNumberArg, suppress_VT_CSI),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"suppress-VT-CSI"
		},
	{ ATF_POINTER, 1, offsetof(struct ProvideRoamingNumberArg, offeredCamel4CSIsInInterrogatingNode),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OfferedCamel4CSIs,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"offeredCamel4CSIsInInterrogatingNode"
		},
};
static ber_tlv_tag_t asn_DEF_ProvideRoamingNumberArg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ProvideRoamingNumberArg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imsi at 203 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* msc-Number at 204 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* msisdn at 205 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* lmsi at 206 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 4, 0, 0 }, /* gsm-BearerCapability at 207 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 5, 0, 0 }, /* networkSignalInfo at 208 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 6, 0, 0 }, /* suppressionOfAnnouncement at 209 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 7, 0, 0 }, /* gmsc-Address at 210 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 8, 0, 0 }, /* callReferenceNumber at 211 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 9, 0, 0 }, /* or-Interrogation at 212 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 10, 0, 0 }, /* extensionContainer at 213 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 11, 0, 0 }, /* alertingPattern at 215 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 12, 0, 0 }, /* ccbs-Call at 216 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* additionalSignalInfo at 218 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 13, 0, 0 }, /* supportedCamelPhasesInInterrogatingNode at 217 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 15, 0, 0 }, /* orNotSupportedInGMSC at 219 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 16, 0, 0 }, /* pre-pagingSupported at 220 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 17, 0, 0 }, /* longFTN-Supported at 221 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 18, 0, 0 }, /* suppress-VT-CSI at 222 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 19, 0, 0 } /* offeredCamel4CSIsInInterrogatingNode at 223 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ProvideRoamingNumberArg_specs_1 = {
	sizeof(struct ProvideRoamingNumberArg),
	offsetof(struct ProvideRoamingNumberArg, _asn_ctx),
	asn_MAP_ProvideRoamingNumberArg_tag2el_1,
	20,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	10,	/* Start extensions */
	21	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ProvideRoamingNumberArg = {
	"ProvideRoamingNumberArg",
	"ProvideRoamingNumberArg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ProvideRoamingNumberArg_tags_1,
	sizeof(asn_DEF_ProvideRoamingNumberArg_tags_1)
		/sizeof(asn_DEF_ProvideRoamingNumberArg_tags_1[0]), /* 1 */
	asn_DEF_ProvideRoamingNumberArg_tags_1,	/* Same as above */
	sizeof(asn_DEF_ProvideRoamingNumberArg_tags_1)
		/sizeof(asn_DEF_ProvideRoamingNumberArg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_ProvideRoamingNumberArg_1,
	20,	/* Elements count */
	&asn_SPC_ProvideRoamingNumberArg_specs_1	/* Additional specs */
};

