#include <asn_internal.h>

#include "LocationInformation.h"

static asn_TYPE_member_t asn_MBR_LocationInformation_1[] = {
	{ ATF_POINTER, 11, offsetof(struct LocationInformation, ageOfLocationInformation),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_AgeOfLocationInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ageOfLocationInformation"
		},
	{ ATF_POINTER, 10, offsetof(struct LocationInformation, geographicalInformation),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeographicalInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"geographicalInformation"
		},
	{ ATF_POINTER, 9, offsetof(struct LocationInformation, vlr_number),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlr-number"
		},
	{ ATF_POINTER, 8, offsetof(struct LocationInformation, locationNumber),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LocationNumber,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"locationNumber"
		},
	{ ATF_POINTER, 7, offsetof(struct LocationInformation, cellGlobalIdOrServiceAreaIdOrLAI),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_CellGlobalIdOrServiceAreaIdOrLAI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cellGlobalIdOrServiceAreaIdOrLAI"
		},
	{ ATF_POINTER, 6, offsetof(struct LocationInformation, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 5, offsetof(struct LocationInformation, selectedLSA_Id),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LSAIdentity,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"selectedLSA-Id"
		},
	{ ATF_POINTER, 4, offsetof(struct LocationInformation, msc_Number),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msc-Number"
		},
	{ ATF_POINTER, 3, offsetof(struct LocationInformation, geodeticInformation),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeodeticInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"geodeticInformation"
		},
	{ ATF_POINTER, 2, offsetof(struct LocationInformation, currentLocationRetrieved),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"currentLocationRetrieved"
		},
	{ ATF_POINTER, 1, offsetof(struct LocationInformation, sai_Present),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sai-Present"
		},
};
static ber_tlv_tag_t asn_DEF_LocationInformation_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_LocationInformation_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* ageOfLocationInformation at 432 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 1, 0, 0 }, /* geographicalInformation at 433 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 2, 0, 0 }, /* vlr-number at 434 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 3, 0, 0 }, /* locationNumber at 435 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 4, 0, 0 }, /* cellGlobalIdOrServiceAreaIdOrLAI at 436 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 5, 0, 0 }, /* extensionContainer at 437 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 6, 0, 0 }, /* selectedLSA-Id at 439 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 7, 0, 0 }, /* msc-Number at 440 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 8, 0, 0 }, /* geodeticInformation at 441 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 9, 0, 0 }, /* currentLocationRetrieved at 442 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 10, 0, 0 } /* sai-Present at 443 */
};
static asn_SEQUENCE_specifics_t asn_SPC_LocationInformation_specs_1 = {
	sizeof(struct LocationInformation),
	offsetof(struct LocationInformation, _asn_ctx),
	asn_MAP_LocationInformation_tag2el_1,
	11,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	5,	/* Start extensions */
	12	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_LocationInformation = {
	"LocationInformation",
	"LocationInformation",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_LocationInformation_tags_1,
	sizeof(asn_DEF_LocationInformation_tags_1)
		/sizeof(asn_DEF_LocationInformation_tags_1[0]), /* 1 */
	asn_DEF_LocationInformation_tags_1,	/* Same as above */
	sizeof(asn_DEF_LocationInformation_tags_1)
		/sizeof(asn_DEF_LocationInformation_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_LocationInformation_1,
	11,	/* Elements count */
	&asn_SPC_LocationInformation_specs_1	/* Additional specs */
};

