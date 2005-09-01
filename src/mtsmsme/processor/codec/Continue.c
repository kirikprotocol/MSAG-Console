#include <asn_internal.h>

#include <Continue.h>

static asn_TYPE_member_t asn_MBR_Continue[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Continue, otid),
    (ASN_TAG_CLASS_APPLICATION | (8 << 2)),
    0,
    (void *)&asn_DEF_OrigTransactionID,
    0,  /* Defer constraints checking to the member type */
    "otid"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Continue, dtid),
    (ASN_TAG_CLASS_APPLICATION | (9 << 2)),
    0,
    (void *)&asn_DEF_DestTransactionID,
    0,  /* Defer constraints checking to the member type */
    "dtid"
    },
  { ATF_POINTER, 2, offsetof(struct Continue, dialoguePortion),
    (ASN_TAG_CLASS_APPLICATION | (11 << 2)),
    0,
    (void *)&asn_DEF_DialoguePortion,
    0,  /* Defer constraints checking to the member type */
    "dialoguePortion"
    },
  { ATF_POINTER, 1, offsetof(struct Continue, componenets),
    (ASN_TAG_CLASS_APPLICATION | (12 << 2)),
    0,
    (void *)&asn_DEF_ComponentPortion,
    0,  /* Defer constraints checking to the member type */
    "componenets"
    },
};
static ber_tlv_tag_t asn_DEF_Continue_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_Continue_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (8 << 2)), 0, 0, 0 }, /* otid at 45 */
    { (ASN_TAG_CLASS_APPLICATION | (9 << 2)), 1, 0, 0 }, /* dtid at 46 */
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 2, 0, 0 }, /* dialoguePortion at 47 */
    { (ASN_TAG_CLASS_APPLICATION | (12 << 2)), 3, 0, 0 }, /* componenets at 48 */
};
static asn_SEQUENCE_specifics_t asn_DEF_Continue_specs = {
  sizeof(struct Continue),
  offsetof(struct Continue, _asn_ctx),
  asn_DEF_Continue_tag2el,
  4,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Continue = {
  "Continue",
  "Continue",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_Continue_tags,
  sizeof(asn_DEF_Continue_tags)
    /sizeof(asn_DEF_Continue_tags[0]), /* 1 */
  asn_DEF_Continue_tags,  /* Same as above */
  sizeof(asn_DEF_Continue_tags)
    /sizeof(asn_DEF_Continue_tags[0]), /* 1 */
  asn_MBR_Continue,
  4,  /* Elements count */
  &asn_DEF_Continue_specs /* Additional specs */
};
