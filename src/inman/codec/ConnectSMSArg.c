#include <asn_internal.h>

#include "ConnectSMSArg.h"

static asn_TYPE_member_t asn_MBR_ConnectSMSArg_1[] = {
	{ ATF_POINTER, 4, offsetof(struct ConnectSMSArg, callingPartysNumber),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SMS_AddressString,
		0,	/* Defer constraints checking to the member type */
		"callingPartysNumber"
		},
	{ ATF_POINTER, 3, offsetof(struct ConnectSMSArg, destinationSubscriberNumber),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_CalledPartyBCDNumber,
		0,	/* Defer constraints checking to the member type */
		"destinationSubscriberNumber"
		},
	{ ATF_POINTER, 2, offsetof(struct ConnectSMSArg, sMSCAddress),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"sMSCAddress"
		},
	{ ATF_POINTER, 1, offsetof(struct ConnectSMSArg, extensions),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Extensions,
		0,	/* Defer constraints checking to the member type */
		"extensions"
		},
};
static ber_tlv_tag_t asn_DEF_ConnectSMSArg_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ConnectSMSArg_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* callingPartysNumber at 1088 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* destinationSubscriberNumber at 1089 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* sMSCAddress at 1090 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 3, 0, 0 } /* extensions at 1091 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ConnectSMSArg_1_specs = {
	sizeof(struct ConnectSMSArg),
	offsetof(struct ConnectSMSArg, _asn_ctx),
	asn_MAP_ConnectSMSArg_1_tag2el,
	4,	/* Count of tags in the map */
	3,	/* Start extensions */
	5	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ConnectSMSArg = {
	"ConnectSMSArg",
	"ConnectSMSArg",
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
	asn_DEF_ConnectSMSArg_1_tags,
	sizeof(asn_DEF_ConnectSMSArg_1_tags)
		/sizeof(asn_DEF_ConnectSMSArg_1_tags[0]), /* 1 */
	asn_DEF_ConnectSMSArg_1_tags,	/* Same as above */
	sizeof(asn_DEF_ConnectSMSArg_1_tags)
		/sizeof(asn_DEF_ConnectSMSArg_1_tags[0]), /* 1 */
	asn_MBR_ConnectSMSArg_1,
	4,	/* Elements count */
	&asn_SPC_ConnectSMSArg_1_specs	/* Additional specs */
};

