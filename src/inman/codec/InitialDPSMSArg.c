#include <asn_internal.h>

#include "InitialDPSMSArg.h"

static asn_TYPE_member_t asn_MBR_InitialDPSMSArg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct InitialDPSMSArg, serviceKey),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		"serviceKey"
		},
	{ ATF_POINTER, 20, offsetof(struct InitialDPSMSArg, destinationSubscriberNumber),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CalledPartyBCDNumber,
		0,	/* Defer constraints checking to the member type */
		"destinationSubscriberNumber"
		},
	{ ATF_POINTER, 19, offsetof(struct InitialDPSMSArg, callingPartyNumber),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SMS_AddressString,
		0,	/* Defer constraints checking to the member type */
		"callingPartyNumber"
		},
	{ ATF_POINTER, 18, offsetof(struct InitialDPSMSArg, eventTypeSMS),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_EventTypeSMS,
		0,	/* Defer constraints checking to the member type */
		"eventTypeSMS"
		},
	{ ATF_POINTER, 17, offsetof(struct InitialDPSMSArg, iMSI),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		"iMSI"
		},
	{ ATF_POINTER, 16, offsetof(struct InitialDPSMSArg, locationInformationMSC),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInformation,
		0,	/* Defer constraints checking to the member type */
		"locationInformationMSC"
		},
	{ ATF_POINTER, 15, offsetof(struct InitialDPSMSArg, locationInformationGPRS),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationInformationGPRS,
		0,	/* Defer constraints checking to the member type */
		"locationInformationGPRS"
		},
	{ ATF_POINTER, 14, offsetof(struct InitialDPSMSArg, sMSCAddress),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"sMSCAddress"
		},
	{ ATF_POINTER, 13, offsetof(struct InitialDPSMSArg, timeAndTimezone),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeAndTimezone,
		0,	/* Defer constraints checking to the member type */
		"timeAndTimezone"
		},
	{ ATF_POINTER, 12, offsetof(struct InitialDPSMSArg, tPShortMessageSpecificInfo),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TPShortMessageSpecificInfo,
		0,	/* Defer constraints checking to the member type */
		"tPShortMessageSpecificInfo"
		},
	{ ATF_POINTER, 11, offsetof(struct InitialDPSMSArg, tPProtocolIdentifier),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TPProtocolIdentifier,
		0,	/* Defer constraints checking to the member type */
		"tPProtocolIdentifier"
		},
	{ ATF_POINTER, 10, offsetof(struct InitialDPSMSArg, tPDataCodingScheme),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TPDataCodingScheme,
		0,	/* Defer constraints checking to the member type */
		"tPDataCodingScheme"
		},
	{ ATF_POINTER, 9, offsetof(struct InitialDPSMSArg, tPValidityPeriod),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TPValidityPeriod,
		0,	/* Defer constraints checking to the member type */
		"tPValidityPeriod"
		},
	{ ATF_POINTER, 8, offsetof(struct InitialDPSMSArg, extensions),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Extensions,
		0,	/* Defer constraints checking to the member type */
		"extensions"
		},
	{ ATF_POINTER, 7, offsetof(struct InitialDPSMSArg, smsReferenceNumber),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CallReferenceNumber,
		0,	/* Defer constraints checking to the member type */
		"smsReferenceNumber"
		},
	{ ATF_POINTER, 6, offsetof(struct InitialDPSMSArg, mscAddress),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"mscAddress"
		},
	{ ATF_POINTER, 5, offsetof(struct InitialDPSMSArg, sgsn_Number),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"sgsn-Number"
		},
	{ ATF_POINTER, 4, offsetof(struct InitialDPSMSArg, ms_Classmark2),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MS_Classmark2,
		0,	/* Defer constraints checking to the member type */
		"ms-Classmark2"
		},
	{ ATF_POINTER, 3, offsetof(struct InitialDPSMSArg, gPRSMSClass),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GPRSMSClass,
		0,	/* Defer constraints checking to the member type */
		"gPRSMSClass"
		},
	{ ATF_POINTER, 2, offsetof(struct InitialDPSMSArg, iMEI),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMEI,
		0,	/* Defer constraints checking to the member type */
		"iMEI"
		},
	{ ATF_POINTER, 1, offsetof(struct InitialDPSMSArg, calledPartyNumber),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"calledPartyNumber"
		},
};
static ber_tlv_tag_t asn_DEF_InitialDPSMSArg_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_InitialDPSMSArg_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* serviceKey at 1060 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* destinationSubscriberNumber at 1061 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* callingPartyNumber at 1062 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* eventTypeSMS at 1063 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* iMSI at 1064 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* locationInformationMSC at 1065 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* locationInformationGPRS at 1066 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* sMSCAddress at 1067 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* timeAndTimezone at 1068 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* tPShortMessageSpecificInfo at 1069 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* tPProtocolIdentifier at 1070 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* tPDataCodingScheme at 1071 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* tPValidityPeriod at 1072 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* extensions at 1073 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* smsReferenceNumber at 1075 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* mscAddress at 1076 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* sgsn-Number at 1077 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* ms-Classmark2 at 1078 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* gPRSMSClass at 1079 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* iMEI at 1080 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 } /* calledPartyNumber at 1081 */
};
static asn_SEQUENCE_specifics_t asn_SPC_InitialDPSMSArg_1_specs = {
	sizeof(struct InitialDPSMSArg),
	offsetof(struct InitialDPSMSArg, _asn_ctx),
	asn_MAP_InitialDPSMSArg_1_tag2el,
	21,	/* Count of tags in the map */
	13,	/* Start extensions */
	22	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_InitialDPSMSArg = {
	"InitialDPSMSArg",
	"InitialDPSMSArg",
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
	asn_DEF_InitialDPSMSArg_1_tags,
	sizeof(asn_DEF_InitialDPSMSArg_1_tags)
		/sizeof(asn_DEF_InitialDPSMSArg_1_tags[0]), /* 1 */
	asn_DEF_InitialDPSMSArg_1_tags,	/* Same as above */
	sizeof(asn_DEF_InitialDPSMSArg_1_tags)
		/sizeof(asn_DEF_InitialDPSMSArg_1_tags[0]), /* 1 */
	asn_MBR_InitialDPSMSArg_1,
	21,	/* Elements count */
	&asn_SPC_InitialDPSMSArg_1_specs	/* Additional specs */
};

