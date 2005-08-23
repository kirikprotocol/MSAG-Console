#include <asn_internal.h>

#include "CellGlobalIdOrServiceAreaIdOrLAI.h"

static asn_TYPE_member_t asn_MBR_CellGlobalIdOrServiceAreaIdOrLAI_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CellGlobalIdOrServiceAreaIdOrLAI, choice.cellGlobalIdOrServiceAreaIdFixedLength),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_CellGlobalIdOrServiceAreaIdFixedLength,
		0,	/* Defer constraints checking to the member type */
		"cellGlobalIdOrServiceAreaIdFixedLength"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CellGlobalIdOrServiceAreaIdOrLAI, choice.laiFixedLength),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		(void *)&asn_DEF_LAIFixedLength,
		0,	/* Defer constraints checking to the member type */
		"laiFixedLength"
		},
};
static asn_TYPE_tag2member_t asn_MAP_CellGlobalIdOrServiceAreaIdOrLAI_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* cellGlobalIdOrServiceAreaIdFixedLength at 382 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* laiFixedLength at 384 */
};
static asn_CHOICE_specifics_t asn_SPC_CellGlobalIdOrServiceAreaIdOrLAI_1_specs = {
	sizeof(struct CellGlobalIdOrServiceAreaIdOrLAI),
	offsetof(struct CellGlobalIdOrServiceAreaIdOrLAI, _asn_ctx),
	offsetof(struct CellGlobalIdOrServiceAreaIdOrLAI, present),
	sizeof(((struct CellGlobalIdOrServiceAreaIdOrLAI *)0)->present),
	asn_MAP_CellGlobalIdOrServiceAreaIdOrLAI_1_tag2el,
	2,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_CellGlobalIdOrServiceAreaIdOrLAI = {
	"CellGlobalIdOrServiceAreaIdOrLAI",
	"CellGlobalIdOrServiceAreaIdOrLAI",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	asn_MBR_CellGlobalIdOrServiceAreaIdOrLAI_1,
	2,	/* Elements count */
	&asn_SPC_CellGlobalIdOrServiceAreaIdOrLAI_1_specs	/* Additional specs */
};

