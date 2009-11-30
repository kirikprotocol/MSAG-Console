#include <asn_internal.h>

#include "DestinationNumberLengthList.h"

static int
memb_NativeInteger_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 1 && value <= 15)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_TYPE_member_t asn_MBR_DestinationNumberLengthList_1[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NativeInteger,
		memb_NativeInteger_constraint_1,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_DestinationNumberLengthList_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_DestinationNumberLengthList_specs_1 = {
	sizeof(struct DestinationNumberLengthList),
	offsetof(struct DestinationNumberLengthList, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
asn_TYPE_descriptor_t asn_DEF_DestinationNumberLengthList = {
	"DestinationNumberLengthList",
	"DestinationNumberLengthList",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DestinationNumberLengthList_tags_1,
	sizeof(asn_DEF_DestinationNumberLengthList_tags_1)
		/sizeof(asn_DEF_DestinationNumberLengthList_tags_1[0]), /* 1 */
	asn_DEF_DestinationNumberLengthList_tags_1,	/* Same as above */
	sizeof(asn_DEF_DestinationNumberLengthList_tags_1)
		/sizeof(asn_DEF_DestinationNumberLengthList_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DestinationNumberLengthList_1,
	1,	/* Single element */
	&asn_SPC_DestinationNumberLengthList_specs_1	/* Additional specs */
};

