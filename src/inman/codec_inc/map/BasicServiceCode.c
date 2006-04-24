#ident "$Id$"

#include <asn_internal.h>

#include "BasicServiceCode.h"

static asn_TYPE_member_t asn_MBR_BasicServiceCode_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct BasicServiceCode, choice.bearerService),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BearerServiceCode,
		0,	/* Defer constraints checking to the member type */
		"bearerService"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct BasicServiceCode, choice.teleservice),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TeleserviceCode,
		0,	/* Defer constraints checking to the member type */
		"teleservice"
		},
};
static asn_TYPE_tag2member_t asn_MAP_BasicServiceCode_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* bearerService at 263 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 } /* teleservice at 265 */
};
static asn_CHOICE_specifics_t asn_SPC_BasicServiceCode_1_specs = {
	sizeof(struct BasicServiceCode),
	offsetof(struct BasicServiceCode, _asn_ctx),
	offsetof(struct BasicServiceCode, present),
	sizeof(((struct BasicServiceCode *)0)->present),
	asn_MAP_BasicServiceCode_1_tag2el,
	2,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_BasicServiceCode = {
	"BasicServiceCode",
	"BasicServiceCode",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
#ifndef ASN1_XER_NOT_USED
	CHOICE_decode_xer,
	CHOICE_encode_xer,
#else  /* ASN1_XER_NOT_USED */
	0, 0,
#endif /* ASN1_XER_NOT_USED */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	asn_MBR_BasicServiceCode_1,
	2,	/* Elements count */
	&asn_SPC_BasicServiceCode_1_specs	/* Additional specs */
};

