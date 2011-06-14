/* #ident "$Id$" */

#include <asn_internal.h>

#include "SystemFailureParam.h"

static asn_TYPE_member_t asn_MBR_SystemFailureParam_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SystemFailureParam, choice.networkResource),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_NetworkResource,
		0,	/* Defer constraints checking to the member type */
		"networkResource"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SystemFailureParam, choice.extensibleSystemFailureParam),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensibleSystemFailureParam,
		0,	/* Defer constraints checking to the member type */
		"extensibleSystemFailureParam"
		},
};
static asn_TYPE_tag2member_t asn_MAP_SystemFailureParam_1_tag2el[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* networkResource at 67 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* extensibleSystemFailureParam at 71 */
};
static asn_CHOICE_specifics_t asn_SPC_SystemFailureParam_1_specs = {
	sizeof(struct SystemFailureParam),
	offsetof(struct SystemFailureParam, _asn_ctx),
	offsetof(struct SystemFailureParam, present),
	sizeof(((struct SystemFailureParam *)0)->present),
	asn_MAP_SystemFailureParam_1_tag2el,
	2,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_SystemFailureParam = {
	"SystemFailureParam",
	"SystemFailureParam",
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
	asn_MBR_SystemFailureParam_1,
	2,	/* Elements count */
	&asn_SPC_SystemFailureParam_1_specs	/* Additional specs */
};

