#ident "$Id$"

#include <asn_internal.h>

#include "LocationInformationGPRScap.h"

static int
memb_cellGlobalIdOrServiceAreaIdOrLAI_1_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_consume_bytes_f *app_errlog, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size >= 5 && size <= 7)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_TYPE_member_t asn_MBR_LocationInformationGPRScap_1[] = {
	{ ATF_POINTER, 7, offsetof(struct LocationInformationGPRScap, cellGlobalIdOrServiceAreaIdOrLAI),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		memb_cellGlobalIdOrServiceAreaIdOrLAI_1_constraint,
		"cellGlobalIdOrServiceAreaIdOrLAI"
		},
	{ ATF_POINTER, 6, offsetof(struct LocationInformationGPRScap, routeingAreaIdentity),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RAIdentity,
		0,	/* Defer constraints checking to the member type */
		"routeingAreaIdentity"
		},
	{ ATF_POINTER, 5, offsetof(struct LocationInformationGPRScap, geographicalInformation),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeographicalInformation,
		0,	/* Defer constraints checking to the member type */
		"geographicalInformation"
		},
	{ ATF_POINTER, 4, offsetof(struct LocationInformationGPRScap, sgsn_Number),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ISDN_AddressString,
		0,	/* Defer constraints checking to the member type */
		"sgsn-Number"
		},
	{ ATF_POINTER, 3, offsetof(struct LocationInformationGPRScap, selectedLSAIdentity),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LSAIdentity,
		0,	/* Defer constraints checking to the member type */
		"selectedLSAIdentity"
		},
	{ ATF_POINTER, 2, offsetof(struct LocationInformationGPRScap, extensionContainer),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ExtensionContainer,
		0,	/* Defer constraints checking to the member type */
		"extensionContainer"
		},
	{ ATF_POINTER, 1, offsetof(struct LocationInformationGPRScap, sai_Present),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		"sai-Present"
		},
};
static ber_tlv_tag_t asn_DEF_LocationInformationGPRScap_1_tags[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_LocationInformationGPRScap_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* cellGlobalIdOrServiceAreaIdOrLAI at 170 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* routeingAreaIdentity at 171 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* geographicalInformation at 172 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* sgsn-Number at 173 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* selectedLSAIdentity at 174 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* extensionContainer at 175 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 } /* sai-Present at 177 */
};
static asn_SEQUENCE_specifics_t asn_SPC_LocationInformationGPRScap_1_specs = {
	sizeof(struct LocationInformationGPRScap),
	offsetof(struct LocationInformationGPRScap, _asn_ctx),
	asn_MAP_LocationInformationGPRScap_1_tag2el,
	7,	/* Count of tags in the map */
	5,	/* Start extensions */
	8	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_LocationInformationGPRScap = {
	"LocationInformationGPRScap",
	"LocationInformationGPRScap",
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
	asn_DEF_LocationInformationGPRScap_1_tags,
	sizeof(asn_DEF_LocationInformationGPRScap_1_tags)
		/sizeof(asn_DEF_LocationInformationGPRScap_1_tags[0]), /* 1 */
	asn_DEF_LocationInformationGPRScap_1_tags,	/* Same as above */
	sizeof(asn_DEF_LocationInformationGPRScap_1_tags)
		/sizeof(asn_DEF_LocationInformationGPRScap_1_tags[0]), /* 1 */
	asn_MBR_LocationInformationGPRScap_1,
	7,	/* Elements count */
	&asn_SPC_LocationInformationGPRScap_1_specs	/* Additional specs */
};

