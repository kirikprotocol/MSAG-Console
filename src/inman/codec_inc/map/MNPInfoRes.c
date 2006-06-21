#ident "$Id$"

#include <asn_internal.h>

#include "MNPInfoRes.h"

static asn_TYPE_member_t asn_MBR_MNPInfoRes_1[] = {
	{ ATF_POINTER, 5, offsetof(struct MNPInfoRes, routeingNumber),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RouteingNumber,
		0,	/* Defer constraints checking to the member type */
		"routeingNumber"
		},
	{ ATF_POINTER, 4, offsetof(struct MNPInfoRes, imsi),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		"imsi"
		},
	{ ATF_POINTER, 3, offsetof(struct MNPInfoRes, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"msisdn"
		},
	{ ATF_POINTER, 2, offsetof(struct MNPInfoRes, numberPortabilityStatus),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NumberPortabilityStatus,
		0,	/* Defer constraints checking to the member type */
		"numberPortabilityStatus"
		},
	{ ATF_POINTER, 1, offsetof(struct MNPInfoRes, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_MNPInfoRes_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MNPInfoRes_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* routeingNumber at 383 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* imsi at 384 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* msisdn at 385 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* numberPortabilityStatus at 386 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* extensionContainer at 387 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MNPInfoRes_1_specs = {
	sizeof(struct MNPInfoRes),
	offsetof(struct MNPInfoRes, _asn_ctx),
	asn_MAP_MNPInfoRes_1_tag2el,
	5,	/* Count of tags in the map */
	4,	/* Start extensions */
	6	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MNPInfoRes = {
	"MNPInfoRes",
	"MNPInfoRes",
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
	asn_DEF_MNPInfoRes_1_tags,
	sizeof(asn_DEF_MNPInfoRes_1_tags)
		/sizeof(asn_DEF_MNPInfoRes_1_tags[0]), /* 1 */
	asn_DEF_MNPInfoRes_1_tags,	/* Same as above */
	sizeof(asn_DEF_MNPInfoRes_1_tags)
		/sizeof(asn_DEF_MNPInfoRes_1_tags[0]), /* 1 */
	asn_MBR_MNPInfoRes_1,
	5,	/* Elements count */
	&asn_SPC_MNPInfoRes_1_specs	/* Additional specs */
};

