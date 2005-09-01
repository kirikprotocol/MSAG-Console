#include <asn_internal.h>

#include <Begin.h>

static asn_TYPE_member_t asn_MBR_Begin[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Begin, otid),
    (ASN_TAG_CLASS_APPLICATION | (8 << 2)),
    0,
    (void *)&asn_DEF_OrigTransactionID,
    0,  /* Defer constraints checking to the member type */
    "otid"
    },
  { ATF_POINTER, 2, offsetof(struct Begin, dialoguePortion),
    (ASN_TAG_CLASS_APPLICATION | (11 << 2)),
    0,
    (void *)&asn_DEF_DialoguePortion,
    0,  /* Defer constraints checking to the member type */
    "dialoguePortion"
    },
  { ATF_POINTER, 1, offsetof(struct Begin, components),
    (ASN_TAG_CLASS_APPLICATION | (12 << 2)),
    0,
    (void *)&asn_DEF_ComponentPortion,
    0,  /* Defer constraints checking to the member type */
    "components"
    },
};
static ber_tlv_tag_t asn_DEF_Begin_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_Begin_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (8 << 2)), 0, 0, 0 }, /* otid at 33 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 1, 0, 0 }, /* dialoguePortion at 34 */
    { (ASN_TAG_CLASS_APPLICATION | (12 << 2)), 2, 0, 0 }, /* components at 35 */
};
static asn_SEQUENCE_specifics_t asn_DEF_Begin_specs = {
  sizeof(struct Begin),
  offsetof(struct Begin, _asn_ctx),
  asn_DEF_Begin_tag2el,
  3,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Begin = {
  "Begin",
  "Begin",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_Begin_tags,
  sizeof(asn_DEF_Begin_tags)
    /sizeof(asn_DEF_Begin_tags[0]), /* 1 */
  asn_DEF_Begin_tags, /* Same as above */
  sizeof(asn_DEF_Begin_tags)
    /sizeof(asn_DEF_Begin_tags[0]), /* 1 */
  asn_MBR_Begin,
  3,  /* Elements count */
  &asn_DEF_Begin_specs  /* Additional specs */
};
