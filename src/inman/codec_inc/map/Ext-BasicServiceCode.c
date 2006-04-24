#ident "$Id$"

#include <asn_internal.h>

#include "Ext-BasicServiceCode.h"

static asn_TYPE_member_t asn_MBR_Ext_BasicServiceCode_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Ext_BasicServiceCode, choice.ext_BearerService),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_BearerServiceCode,
		0,	/* Defer constraints checking to the member type */
		"ext-BearerService"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Ext_BasicServiceCode, choice.ext_Teleservice),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Ext_TeleserviceCode,
		0,	/* Defer constraints checking to the member type */
		"ext-Teleservice"
		},
};
static asn_TYPE_tag2member_t asn_MAP_Ext_BasicServiceCode_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* ext-BearerService at 267 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 1, 0, 0 } /* ext-Teleservice at 269 */
};
static asn_CHOICE_specifics_t asn_SPC_Ext_BasicServiceCode_1_specs = {
	sizeof(struct Ext_BasicServiceCode),
	offsetof(struct Ext_BasicServiceCode, _asn_ctx),
	offsetof(struct Ext_BasicServiceCode, present),
	sizeof(((struct Ext_BasicServiceCode *)0)->present),
	asn_MAP_Ext_BasicServiceCode_1_tag2el,
	2,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_Ext_BasicServiceCode = {
	"Ext-BasicServiceCode",
	"Ext-BasicServiceCode",
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
	asn_MBR_Ext_BasicServiceCode_1,
	2,	/* Elements count */
	&asn_SPC_Ext_BasicServiceCode_1_specs	/* Additional specs */
};

