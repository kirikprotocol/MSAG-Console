#ident "$Id$"

#include <asn_internal.h>

#include "CAMEL-FCISMSBillingChargingCharacteristics.h"

static int
memb_freeFormatData_2_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
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
	
	if((size >= 1 && size <= 160)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_TYPE_member_t asn_MBR_fCIBCCCAMELsequence1_2[] = {
	{ ATF_NOFLAGS, 0, offsetof(fCIBCCCAMELsequence1_t, freeFormatData),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		memb_freeFormatData_2_constraint,
		"freeFormatData"
		},
	{ ATF_POINTER, 1, offsetof(fCIBCCCAMELsequence1_t, appendFreeFormatData),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AppendFreeFormatData,
		0,	/* Defer constraints checking to the member type */
		"appendFreeFormatData"
		},
};
static ber_tlv_tag_t asn_DEF_fCIBCCCAMELsequence1_2_tags[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_fCIBCCCAMELsequence1_2_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* freeFormatData at 104 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* appendFreeFormatData at 107 */
};
static asn_SEQUENCE_specifics_t asn_SPC_fCIBCCCAMELsequence1_2_specs = {
	sizeof(fCIBCCCAMELsequence1_t),
	offsetof(fCIBCCCAMELsequence1_t, _asn_ctx),
	asn_MAP_fCIBCCCAMELsequence1_2_tag2el,
	2,	/* Count of tags in the map */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_fCIBCCCAMELsequence1_2 = {
	"fCIBCCCAMELsequence1",
	"fCIBCCCAMELsequence1",
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
	asn_DEF_fCIBCCCAMELsequence1_2_tags,
	sizeof(asn_DEF_fCIBCCCAMELsequence1_2_tags)
		/sizeof(asn_DEF_fCIBCCCAMELsequence1_2_tags[0]) - 1, /* 1 */
	asn_DEF_fCIBCCCAMELsequence1_2_tags,	/* Same as above */
	sizeof(asn_DEF_fCIBCCCAMELsequence1_2_tags)
		/sizeof(asn_DEF_fCIBCCCAMELsequence1_2_tags[0]), /* 2 */
	asn_MBR_fCIBCCCAMELsequence1_2,
	2,	/* Elements count */
	&asn_SPC_fCIBCCCAMELsequence1_2_specs	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_CAMEL_FCISMSBillingChargingCharacteristics_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CAMEL_FCISMSBillingChargingCharacteristics, choice.fCIBCCCAMELsequence1),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_fCIBCCCAMELsequence1_2,
		0,	/* Defer constraints checking to the member type */
		"fCIBCCCAMELsequence1"
		},
};
static asn_TYPE_tag2member_t asn_MAP_CAMEL_FCISMSBillingChargingCharacteristics_1_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* fCIBCCCAMELsequence1 at 106 */
};
static asn_CHOICE_specifics_t asn_SPC_CAMEL_FCISMSBillingChargingCharacteristics_1_specs = {
	sizeof(struct CAMEL_FCISMSBillingChargingCharacteristics),
	offsetof(struct CAMEL_FCISMSBillingChargingCharacteristics, _asn_ctx),
	offsetof(struct CAMEL_FCISMSBillingChargingCharacteristics, present),
	sizeof(((struct CAMEL_FCISMSBillingChargingCharacteristics *)0)->present),
	asn_MAP_CAMEL_FCISMSBillingChargingCharacteristics_1_tag2el,
	1,	/* Count of tags in the map */
	0	/* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_CAMEL_FCISMSBillingChargingCharacteristics = {
	"CAMEL-FCISMSBillingChargingCharacteristics",
	"CAMEL-FCISMSBillingChargingCharacteristics",
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
	asn_MBR_CAMEL_FCISMSBillingChargingCharacteristics_1,
	1,	/* Elements count */
	&asn_SPC_CAMEL_FCISMSBillingChargingCharacteristics_1_specs	/* Additional specs */
};

