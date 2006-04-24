#ident "$Id$"

#include <asn_internal.h>

#include "SMS-CSI.h"

static asn_TYPE_member_t asn_MBR_SMS_CSI_1[] = {
	{ ATF_POINTER, 5, offsetof(struct SMS_CSI, sms_CAMEL_TDP_DataList),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SMS_CAMEL_TDP_DataList,
		0,	/* Defer constraints checking to the member type */
		"sms-CAMEL-TDP-DataList"
		},
	{ ATF_POINTER, 4, offsetof(struct SMS_CSI, camelCapabilityHandling),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CamelCapabilityHandling,
		0,	/* Defer constraints checking to the member type */
		"camelCapabilityHandling"
		},
	{ ATF_POINTER, 3, offsetof(struct SMS_CSI, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 2, offsetof(struct SMS_CSI, notificationToCSE),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"notificationToCSE"
		},
	{ ATF_POINTER, 1, offsetof(struct SMS_CSI, csi_Active),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"csi-Active"
		},
};
static ber_tlv_tag_t asn_DEF_SMS_CSI_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SMS_CSI_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* sms-CAMEL-TDP-DataList at 611 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* camelCapabilityHandling at 612 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* extensionContainer at 613 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* notificationToCSE at 614 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* csi-Active at 615 */
};
static asn_SEQUENCE_specifics_t asn_SPC_SMS_CSI_1_specs = {
	sizeof(struct SMS_CSI),
	offsetof(struct SMS_CSI, _asn_ctx),
	asn_MAP_SMS_CSI_1_tag2el,
	5,	/* Count of tags in the map */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SMS_CSI = {
	"SMS-CSI",
	"SMS-CSI",
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
	asn_DEF_SMS_CSI_1_tags,
	sizeof(asn_DEF_SMS_CSI_1_tags)
		/sizeof(asn_DEF_SMS_CSI_1_tags[0]), /* 1 */
	asn_DEF_SMS_CSI_1_tags,	/* Same as above */
	sizeof(asn_DEF_SMS_CSI_1_tags)
		/sizeof(asn_DEF_SMS_CSI_1_tags[0]), /* 1 */
	asn_MBR_SMS_CSI_1,
	5,	/* Elements count */
	&asn_SPC_SMS_CSI_1_specs	/* Additional specs */
};

