#include <asn_internal.h>

#include "MNPInfoRes.h"

static asn_TYPE_member_t asn_MBR_MNPInfoRes_1[] = {
	{ ATF_POINTER, 5, offsetof(struct MNPInfoRes, routeingNumber),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RouteingNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"routeingNumber"
		},
	{ ATF_POINTER, 4, offsetof(struct MNPInfoRes, imsi),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_IMSI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_POINTER, 3, offsetof(struct MNPInfoRes, msisdn),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_POINTER, 2, offsetof(struct MNPInfoRes, numberPortabilityStatus),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NumberPortabilityStatus,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"numberPortabilityStatus"
		},
	{ ATF_POINTER, 1, offsetof(struct MNPInfoRes, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
};
static ber_tlv_tag_t asn_DEF_MNPInfoRes_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MNPInfoRes_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* routeingNumber at 451 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* imsi at 452 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* msisdn at 453 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* numberPortabilityStatus at 454 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* extensionContainer at 455 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MNPInfoRes_specs_1 = {
	sizeof(struct MNPInfoRes),
	offsetof(struct MNPInfoRes, _asn_ctx),
	asn_MAP_MNPInfoRes_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
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
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_MNPInfoRes_tags_1,
	sizeof(asn_DEF_MNPInfoRes_tags_1)
		/sizeof(asn_DEF_MNPInfoRes_tags_1[0]), /* 1 */
	asn_DEF_MNPInfoRes_tags_1,	/* Same as above */
	sizeof(asn_DEF_MNPInfoRes_tags_1)
		/sizeof(asn_DEF_MNPInfoRes_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_MNPInfoRes_1,
	5,	/* Elements count */
	&asn_SPC_MNPInfoRes_specs_1	/* Additional specs */
};

