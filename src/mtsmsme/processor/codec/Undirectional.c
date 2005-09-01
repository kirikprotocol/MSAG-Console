#include <asn_internal.h>

#include <Undirectional.h>

static asn_TYPE_member_t asn_MBR_Undirectional[] = {
  { ATF_POINTER, 1, offsetof(struct Undirectional, dialoguePortion),
    (ASN_TAG_CLASS_APPLICATION | (11 << 2)),
    0,
    (void *)&asn_DEF_DialoguePortion,
    0,  /* Defer constraints checking to the member type */
    "dialoguePortion"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Undirectional, components),
    (ASN_TAG_CLASS_APPLICATION | (12 << 2)),
    0,
    (void *)&asn_DEF_ComponentPortion,
    0,  /* Defer constraints checking to the member type */
    "components"
    },
};
static ber_tlv_tag_t asn_DEF_Undirectional_tags[] = {
  (ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_DEF_Undirectional_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (11 << 2)), 0, 0, 0 }, /* dialoguePortion at 28 */
    { (ASN_TAG_CLASS_APPLICATION | (12 << 2)), 1, 0, 0 }, /* components at 30 */
};
static asn_SEQUENCE_specifics_t asn_DEF_Undirectional_specs = {
  sizeof(struct Undirectional),
  offsetof(struct Undirectional, _asn_ctx),
  asn_DEF_Undirectional_tag2el,
  2,  /* Count of tags in the map */
  -1, /* Start extensions */
  -1  /* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Undirectional = {
  "Undirectional",
  "Undirectional",
  SEQUENCE_free,
  SEQUENCE_print,
  SEQUENCE_constraint,
  SEQUENCE_decode_ber,
  SEQUENCE_encode_der,
  SEQUENCE_decode_xer,
  SEQUENCE_encode_xer,
  0,  /* Use generic outmost tag fetcher */
  asn_DEF_Undirectional_tags,
  sizeof(asn_DEF_Undirectional_tags)
    /sizeof(asn_DEF_Undirectional_tags[0]), /* 1 */
  asn_DEF_Undirectional_tags, /* Same as above */
  sizeof(asn_DEF_Undirectional_tags)
    /sizeof(asn_DEF_Undirectional_tags[0]), /* 1 */
  asn_MBR_Undirectional,
  2,  /* Elements count */
  &asn_DEF_Undirectional_specs  /* Additional specs */
};
