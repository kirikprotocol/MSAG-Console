#include <asn_internal.h>

#include "SendRoutingInfoRes.h"

static asn_TYPE_member_t asn_MBR_SendRoutingInfoRes_1[] = {
	{ ATF_POINTER, 24, offsetof(struct SendRoutingInfoRes, imsi),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_POINTER, 23, offsetof(struct SendRoutingInfoRes, extendedRoutingInfo),
		(ber_tlv_tag_t)-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_ExtendedRoutingInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extendedRoutingInfo"
		},
	{ ATF_POINTER, 22, offsetof(struct SendRoutingInfoRes, cug_CheckInfo),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CUG_CheckInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cug-CheckInfo"
		},
	{ ATF_POINTER, 21, offsetof(struct SendRoutingInfoRes, cugSubscriptionFlag),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cugSubscriptionFlag"
		},
	{ ATF_POINTER, 20, offsetof(struct SendRoutingInfoRes, subscriberInfo),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SubscriberInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"subscriberInfo"
		},
	{ ATF_POINTER, 19, offsetof(struct SendRoutingInfoRes, ss_List),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SS_List,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ss-List"
		},
	{ ATF_POINTER, 18, offsetof(struct SendRoutingInfoRes, basicService),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_Ext_BasicServiceCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"basicService"
		},
	{ ATF_POINTER, 17, offsetof(struct SendRoutingInfoRes, forwardingInterrogationRequired),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"forwardingInterrogationRequired"
		},
	{ ATF_POINTER, 16, offsetof(struct SendRoutingInfoRes, vmsc_Address),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vmsc-Address"
		},
	{ ATF_POINTER, 15, offsetof(struct SendRoutingInfoRes, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 14, offsetof(struct SendRoutingInfoRes, naea_PreferredCI),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NAEA_PreferredCI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"naea-PreferredCI"
		},
	{ ATF_POINTER, 13, offsetof(struct SendRoutingInfoRes, ccbs_Indicators),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CCBS_Indicators,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ccbs-Indicators"
		},
	{ ATF_POINTER, 12, offsetof(struct SendRoutingInfoRes, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_POINTER, 11, offsetof(struct SendRoutingInfoRes, numberPortabilityStatus),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NumberPortabilityStatus,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"numberPortabilityStatus"
		},
	{ ATF_POINTER, 10, offsetof(struct SendRoutingInfoRes, istAlertTimer),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IST_AlertTimerValue,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"istAlertTimer"
		},
	{ ATF_POINTER, 9, offsetof(struct SendRoutingInfoRes, supportedCamelPhasesInVMSC),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SupportedCamelPhases,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"supportedCamelPhasesInVMSC"
		},
	{ ATF_POINTER, 8, offsetof(struct SendRoutingInfoRes, offeredCamel4CSIsInVMSC),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OfferedCamel4CSIs,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"offeredCamel4CSIsInVMSC"
		},
	{ ATF_POINTER, 7, offsetof(struct SendRoutingInfoRes, routingInfo2),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_RoutingInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"routingInfo2"
		},
	{ ATF_POINTER, 6, offsetof(struct SendRoutingInfoRes, ss_List2),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SS_List,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ss-List2"
		},
	{ ATF_POINTER, 5, offsetof(struct SendRoutingInfoRes, basicService2),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_Ext_BasicServiceCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"basicService2"
		},
	{ ATF_POINTER, 4, offsetof(struct SendRoutingInfoRes, allowedServices),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AllowedServices,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"allowedServices"
		},
	{ ATF_POINTER, 3, offsetof(struct SendRoutingInfoRes, unavailabilityCause),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_UnavailabilityCause,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"unavailabilityCause"
		},
	{ ATF_POINTER, 2, offsetof(struct SendRoutingInfoRes, releaseResourcesSupported),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"releaseResourcesSupported"
		},
	{ ATF_POINTER, 1, offsetof(struct SendRoutingInfoRes, gsm_BearerCapability),
		(ASN_TAG_CLASS_CONTEXT | (23 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExternalSignalInfo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gsm-BearerCapability"
		},
};
static ber_tlv_tag_t asn_DEF_SendRoutingInfoRes_tags_1[] = {
	(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SendRoutingInfoRes_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, 0, 0 }, /* roamingNumber at 188 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 }, /* forwardingData at 189 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 9, 0, 0 }, /* extensionContainer at 143 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 5, 0, 0 }, /* ss-List at 139 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 8, 0, 0 }, /* vmsc-Address at 142 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* cug-CheckInfo at 136 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 7, 0, 0 }, /* forwardingInterrogationRequired at 141 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 6, 0, 0 }, /* basicService at 140 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 3, 0, 0 }, /* cugSubscriptionFlag at 137 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 4, 0, 0 }, /* subscriberInfo at 138 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 1, 0, 0 }, /* camelRoutingInfo at 242 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 0, 0, 0 }, /* imsi at 131 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* naea-PreferredCI at 145 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* ccbs-Indicators at 147 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* msisdn at 148 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* numberPortabilityStatus at 149 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* istAlertTimer at 150 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* supportedCamelPhasesInVMSC at 151 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* offeredCamel4CSIsInVMSC at 152 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* routingInfo2 at 153 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* ss-List2 at 154 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* basicService2 at 155 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* allowedServices at 156 */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* unavailabilityCause at 157 */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 }, /* releaseResourcesSupported at 158 */
    { (ASN_TAG_CLASS_CONTEXT | (23 << 2)), 23, 0, 0 } /* gsm-BearerCapability at 159 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SendRoutingInfoRes_specs_1 = {
	sizeof(struct SendRoutingInfoRes),
	offsetof(struct SendRoutingInfoRes, _asn_ctx),
	asn_MAP_SendRoutingInfoRes_tag2el_1,
	26,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	9,	/* Start extensions */
	25	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SendRoutingInfoRes = {
	"SendRoutingInfoRes",
	"SendRoutingInfoRes",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SendRoutingInfoRes_tags_1,
	sizeof(asn_DEF_SendRoutingInfoRes_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoRes_tags_1[0]) - 1, /* 1 */
	asn_DEF_SendRoutingInfoRes_tags_1,	/* Same as above */
	sizeof(asn_DEF_SendRoutingInfoRes_tags_1)
		/sizeof(asn_DEF_SendRoutingInfoRes_tags_1[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_SendRoutingInfoRes_1,
	24,	/* Elements count */
	&asn_SPC_SendRoutingInfoRes_specs_1	/* Additional specs */
};

