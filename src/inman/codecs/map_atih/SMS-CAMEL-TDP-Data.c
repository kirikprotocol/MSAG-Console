#ident "$Id$"

#include <asn_internal.h>

#include "SMS-CAMEL-TDP-Data.h"

static asn_TYPE_member_t asn_MBR_SMS_CAMEL_TDP_Data_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SMS_CAMEL_TDP_Data, sms_TriggerDetectionPoint),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SMS_TriggerDetectionPoint,
		0,	/* Defer constraints checking to the member type */
		"sms-TriggerDetectionPoint"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMS_CAMEL_TDP_Data, serviceKey),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ServiceKey,
		0,	/* Defer constraints checking to the member type */
		"serviceKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMS_CAMEL_TDP_Data, gsmSCF_Address),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"gsmSCF-Address"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SMS_CAMEL_TDP_Data, defaultSMS_Handling),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DefaultSMS_Handling,
		0,	/* Defer constraints checking to the member type */
		"defaultSMS-Handling"
		},
	{ ATF_POINTER, 1, offsetof(struct SMS_CAMEL_TDP_Data, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_SMS_CAMEL_TDP_Data_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SMS_CAMEL_TDP_Data_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sms-TriggerDetectionPoint at 632 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* serviceKey at 633 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* gsmSCF-Address at 634 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* defaultSMS-Handling at 635 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* extensionContainer at 636 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SMS_CAMEL_TDP_Data_1_specs = {
	sizeof(struct SMS_CAMEL_TDP_Data),
	offsetof(struct SMS_CAMEL_TDP_Data, _asn_ctx),
	asn_MAP_SMS_CAMEL_TDP_Data_1_tag2el,
	5,	/* Count of tags in the map */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SMS_CAMEL_TDP_Data = {
	"SMS-CAMEL-TDP-Data",
	"SMS-CAMEL-TDP-Data",
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
	asn_DEF_SMS_CAMEL_TDP_Data_1_tags,
	sizeof(asn_DEF_SMS_CAMEL_TDP_Data_1_tags)
		/sizeof(asn_DEF_SMS_CAMEL_TDP_Data_1_tags[0]), /* 1 */
	asn_DEF_SMS_CAMEL_TDP_Data_1_tags,	/* Same as above */
	sizeof(asn_DEF_SMS_CAMEL_TDP_Data_1_tags)
		/sizeof(asn_DEF_SMS_CAMEL_TDP_Data_1_tags[0]), /* 1 */
	asn_MBR_SMS_CAMEL_TDP_Data_1,
	5,	/* Elements count */
	&asn_SPC_SMS_CAMEL_TDP_Data_1_specs	/* Additional specs */
};

