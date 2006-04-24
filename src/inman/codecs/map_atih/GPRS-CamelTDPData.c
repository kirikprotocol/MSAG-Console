#ident "$Id$"

#include <asn_internal.h>

#include "GPRS-CamelTDPData.h"

static asn_TYPE_member_t asn_MBR_GPRS_CamelTDPData_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct GPRS_CamelTDPData, gprs_TriggerDetectionPoint),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GPRS_TriggerDetectionPoint,
		0,	/* Defer constraints checking to the member type */
		"gprs-TriggerDetectionPoint"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct GPRS_CamelTDPData, serviceKey),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		"serviceKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct GPRS_CamelTDPData, gsmSCF_Address),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"gsmSCF-Address"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct GPRS_CamelTDPData, defaultSessionHandling),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DefaultGPRS_Handling,
		0,	/* Defer constraints checking to the member type */
		"defaultSessionHandling"
		},
	{ ATF_POINTER, 1, offsetof(struct GPRS_CamelTDPData, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_GPRS_CamelTDPData_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_GPRS_CamelTDPData_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* gprs-TriggerDetectionPoint at 240 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* serviceKey at 241 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* gsmSCF-Address at 242 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* defaultSessionHandling at 243 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* extensionContainer at 244 */
};
static asn_SEQUENCE_specifics_t asn_SPC_GPRS_CamelTDPData_1_specs = {
	sizeof(struct GPRS_CamelTDPData),
	offsetof(struct GPRS_CamelTDPData, _asn_ctx),
	asn_MAP_GPRS_CamelTDPData_1_tag2el,
	5,	/* Count of tags in the map */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_GPRS_CamelTDPData = {
	"GPRS-CamelTDPData",
	"GPRS-CamelTDPData",
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
	asn_DEF_GPRS_CamelTDPData_1_tags,
	sizeof(asn_DEF_GPRS_CamelTDPData_1_tags)
		/sizeof(asn_DEF_GPRS_CamelTDPData_1_tags[0]), /* 1 */
	asn_DEF_GPRS_CamelTDPData_1_tags,	/* Same as above */
	sizeof(asn_DEF_GPRS_CamelTDPData_1_tags)
		/sizeof(asn_DEF_GPRS_CamelTDPData_1_tags[0]), /* 1 */
	asn_MBR_GPRS_CamelTDPData_1,
	5,	/* Elements count */
	&asn_SPC_GPRS_CamelTDPData_1_specs	/* Additional specs */
};

