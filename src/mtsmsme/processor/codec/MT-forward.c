#include <asn_internal.h>

#include "MT-forward.h"

static int
memb_imsi_constraint_2(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size >= 3 && size <= 8)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_lmsi_constraint_2(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size == 4)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_serviceCentreAddressDA_constraint_2(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size >= 1 && size <= 20)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_msisdn_constraint_7(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size >= 1 && size <= 9)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_serviceCentreAddressOA_constraint_7(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size >= 1 && size <= 20)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_sm_RP_UI_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OCTET_STRING_t *st = (const OCTET_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size >= 1 && size <= 200)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_TYPE_member_t asn_MBR_sm_RP_DA_2[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.imsi),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		memb_imsi_constraint_2,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"imsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.lmsi),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		memb_lmsi_constraint_2,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lmsi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.serviceCentreAddressDA),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		memb_serviceCentreAddressDA_constraint_2,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceCentreAddressDA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct sm_RP_DA, choice.noSM_RP_DA),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"noSM-RP-DA"
		},
};
static asn_TYPE_tag2member_t asn_MAP_sm_RP_DA_tag2el_2[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imsi at 202 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* lmsi at 203 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 2, 0, 0 }, /* serviceCentreAddressDA at 204 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 3, 0, 0 } /* noSM-RP-DA at 205 */
};
static asn_CHOICE_specifics_t asn_SPC_sm_RP_DA_specs_2 = {
	sizeof(struct sm_RP_DA),
	offsetof(struct sm_RP_DA, _asn_ctx),
	offsetof(struct sm_RP_DA, present),
	sizeof(((struct sm_RP_DA *)0)->present),
	asn_MAP_sm_RP_DA_tag2el_2,
	4,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_sm_RP_DA_2 = {
	"sm-RP-DA",
	"sm-RP-DA",
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
	asn_MBR_sm_RP_DA_2,
	4,	/* Elements count */
	&asn_SPC_sm_RP_DA_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_sm_RP_OA_7[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct sm_RP_OA, choice.msisdn),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		memb_msisdn_constraint_7,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"msisdn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct sm_RP_OA, choice.serviceCentreAddressOA),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		memb_serviceCentreAddressOA_constraint_7,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"serviceCentreAddressOA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct sm_RP_OA, choice.noSM_RP_OA),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"noSM-RP-OA"
		},
};
static asn_TYPE_tag2member_t asn_MAP_sm_RP_OA_tag2el_7[] = {
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 0, 0, 0 }, /* msisdn at 208 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, 0, 0 }, /* serviceCentreAddressOA at 209 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 2, 0, 0 } /* noSM-RP-OA at 210 */
};
static asn_CHOICE_specifics_t asn_SPC_sm_RP_OA_specs_7 = {
	sizeof(struct sm_RP_OA),
	offsetof(struct sm_RP_OA, _asn_ctx),
	offsetof(struct sm_RP_OA, present),
	sizeof(((struct sm_RP_OA *)0)->present),
	asn_MAP_sm_RP_OA_tag2el_7,
	3,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_sm_RP_OA_7 = {
	"sm-RP-OA",
	"sm-RP-OA",
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
	asn_MBR_sm_RP_OA_7,
	3,	/* Elements count */
	&asn_SPC_sm_RP_OA_specs_7	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_MT_forward_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct MT_forward, sm_RP_DA),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_sm_RP_DA_2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-DA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MT_forward, sm_RP_OA),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_sm_RP_OA_7,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-OA"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MT_forward, sm_RP_UI),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_OCTET_STRING,
		memb_sm_RP_UI_constraint_1,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sm-RP-UI"
		},
	{ ATF_POINTER, 1, offsetof(struct MT_forward, moreMessagesToSend),
		(ASN_TAG_CLASS_UNIVERSAL | (5 << 2)),
		0,
		&asn_DEF_NULL,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"moreMessagesToSend"
		},
};
static ber_tlv_tag_t asn_DEF_MT_forward_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MT_forward_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, 0, 0 }, /* sm-RP-UI at 212 */
    { (ASN_TAG_CLASS_UNIVERSAL | (5 << 2)), 3, 0, 0 }, /* moreMessagesToSend at 213 */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* imsi at 202 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* lmsi at 203 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* msisdn at 208 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 0, 0, 1 }, /* serviceCentreAddressDA at 204 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 1, -1, 0 }, /* serviceCentreAddressOA at 209 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 0, 0, 1 }, /* noSM-RP-DA at 205 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 1, -1, 0 } /* noSM-RP-OA at 210 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MT_forward_specs_1 = {
	sizeof(struct MT_forward),
	offsetof(struct MT_forward, _asn_ctx),
	asn_MAP_MT_forward_tag2el_1,
	9,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MT_forward = {
	"MT-forward",
	"MT-forward",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_MT_forward_tags_1,
	sizeof(asn_DEF_MT_forward_tags_1)
		/sizeof(asn_DEF_MT_forward_tags_1[0]), /* 1 */
	asn_DEF_MT_forward_tags_1,	/* Same as above */
	sizeof(asn_DEF_MT_forward_tags_1)
		/sizeof(asn_DEF_MT_forward_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_MT_forward_1,
	4,	/* Elements count */
	&asn_SPC_MT_forward_specs_1	/* Additional specs */
};

