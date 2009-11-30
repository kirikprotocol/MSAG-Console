#include <asn_internal.h>

#include "LocationInformationGPRS.h"

static asn_TYPE_member_t asn_MBR_LocationInformationGPRS_1[] = {
	{ ATF_POINTER, 10, offsetof(struct LocationInformationGPRS, cellGlobalIdOrServiceAreaIdOrLAI),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_CellGlobalIdOrServiceAreaIdOrLAI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cellGlobalIdOrServiceAreaIdOrLAI"
		},
	{ ATF_POINTER, 9, offsetof(struct LocationInformationGPRS, routeingAreaIdentity),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RAIdentity,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"routeingAreaIdentity"
		},
	{ ATF_POINTER, 8, offsetof(struct LocationInformationGPRS, geographicalInformation),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeographicalInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"geographicalInformation"
		},
	{ ATF_POINTER, 7, offsetof(struct LocationInformationGPRS, sgsn_Number),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sgsn-Number"
		},
	{ ATF_POINTER, 6, offsetof(struct LocationInformationGPRS, selectedLSAIdentity),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LSAIdentity,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"selectedLSAIdentity"
		},
	{ ATF_POINTER, 5, offsetof(struct LocationInformationGPRS, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensionContainer"
		},
	{ ATF_POINTER, 4, offsetof(struct LocationInformationGPRS, sai_Present),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sai-Present"
		},
	{ ATF_POINTER, 3, offsetof(struct LocationInformationGPRS, geodeticInformation),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeodeticInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"geodeticInformation"
		},
	{ ATF_POINTER, 2, offsetof(struct LocationInformationGPRS, currentLocationRetrieved),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"currentLocationRetrieved"
		},
	{ ATF_POINTER, 1, offsetof(struct LocationInformationGPRS, ageOfLocationInformation),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AgeOfLocationInformation,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ageOfLocationInformation"
		},
};
static ber_tlv_tag_t asn_DEF_LocationInformationGPRS_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_LocationInformationGPRS_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* cellGlobalIdOrServiceAreaIdOrLAI at 496 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* routeingAreaIdentity at 497 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* geographicalInformation at 498 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* sgsn-Number at 499 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* selectedLSAIdentity at 500 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* extensionContainer at 501 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* sai-Present at 503 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* geodeticInformation at 504 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* currentLocationRetrieved at 505 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 } /* ageOfLocationInformation at 506 */
};
static asn_SEQUENCE_specifics_t asn_SPC_LocationInformationGPRS_specs_1 = {
	sizeof(struct LocationInformationGPRS),
	offsetof(struct LocationInformationGPRS, _asn_ctx),
	asn_MAP_LocationInformationGPRS_tag2el_1,
	10,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	5,	/* Start extensions */
	11	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_LocationInformationGPRS = {
	"LocationInformationGPRS",
	"LocationInformationGPRS",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_LocationInformationGPRS_tags_1,
	sizeof(asn_DEF_LocationInformationGPRS_tags_1)
		/sizeof(asn_DEF_LocationInformationGPRS_tags_1[0]), /* 1 */
	asn_DEF_LocationInformationGPRS_tags_1,	/* Same as above */
	sizeof(asn_DEF_LocationInformationGPRS_tags_1)
		/sizeof(asn_DEF_LocationInformationGPRS_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_LocationInformationGPRS_1,
	10,	/* Elements count */
	&asn_SPC_LocationInformationGPRS_specs_1	/* Additional specs */
};

