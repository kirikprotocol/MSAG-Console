#include <asn_internal.h>

#include "SystemFailureParam.h"

static asn_TYPE_member_t asn_MBR_SystemFailureParam_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SystemFailureParam, choice.networkResource),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_NetworkResource,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"networkResource"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SystemFailureParam, choice.extensibleSystemFailureParam),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_ExtensibleSystemFailureParam,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"extensibleSystemFailureParam"
		},
};
static asn_TYPE_tag2member_t asn_MAP_SystemFailureParam_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 0, 0, 0 }, /* networkResource at 115 */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, 0, 0 } /* extensibleSystemFailureParam at 119 */
};
static asn_CHOICE_specifics_t asn_SPC_SystemFailureParam_specs_1 = {
	sizeof(struct SystemFailureParam),
	offsetof(struct SystemFailureParam, _asn_ctx),
	offsetof(struct SystemFailureParam, present),
	sizeof(((struct SystemFailureParam *)0)->present),
	asn_MAP_SystemFailureParam_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_SystemFailureParam = {
	"SystemFailureParam",
	"SystemFailureParam",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_SystemFailureParam_1,
	2,	/* Elements count */
	&asn_SPC_SystemFailureParam_specs_1	/* Additional specs */
};

