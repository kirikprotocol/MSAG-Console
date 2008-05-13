#include <asn_internal.h>

#include "Component.h"

static asn_TYPE_member_t asn_MBR_Component_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Component, choice.invoke),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Invoke,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"invoke"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Component, choice.returnResultLast),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ReturnResult,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"returnResultLast"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Component, choice.returnError),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ReturnError,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"returnError"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Component, choice.reject),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Reject,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"reject"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Component, choice.returnResultNotLast),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ReturnResult,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"returnResultNotLast"
		},
};
static asn_TYPE_tag2member_t asn_MAP_Component_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* invoke at 97 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* returnResultLast at 98 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* returnError at 99 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* reject at 100 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 4, 0, 0 } /* returnResultNotLast at 102 */
};
static asn_CHOICE_specifics_t asn_SPC_Component_specs_1 = {
	sizeof(struct Component),
	offsetof(struct Component, _asn_ctx),
	offsetof(struct Component, present),
	sizeof(((struct Component *)0)->present),
	asn_MAP_Component_tag2el_1,
	5,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_Component = {
	"Component",
	"Component",
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
	asn_MBR_Component_1,
	5,	/* Elements count */
	&asn_SPC_Component_specs_1	/* Additional specs */
};

