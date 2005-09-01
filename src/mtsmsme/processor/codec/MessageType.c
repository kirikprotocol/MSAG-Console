#include <asn_internal.h>

#include <MessageType.h>

static asn_TYPE_member_t asn_MBR_MessageType[] = {
  { ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.undirectional),
    (ASN_TAG_CLASS_APPLICATION | (1 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_Undirectional,
    0,  /* Defer constraints checking to the member type */
    "undirectional"
    },
  { ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.begin),
    (ASN_TAG_CLASS_APPLICATION | (2 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_Begin,
    0,  /* Defer constraints checking to the member type */
    "begin"
    },
  { ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.end),
    (ASN_TAG_CLASS_APPLICATION | (4 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_End,
    0,  /* Defer constraints checking to the member type */
    "end"
    },
  { ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.contiinue),
    (ASN_TAG_CLASS_APPLICATION | (5 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_Continue,
    0,  /* Defer constraints checking to the member type */
    "contiinue"
    },
  { ATF_NOFLAGS, 0, offsetof(struct MessageType, choice.abort),
    (ASN_TAG_CLASS_APPLICATION | (7 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_Abort,
    0,  /* Defer constraints checking to the member type */
    "abort"
    },
};
static asn_TYPE_tag2member_t asn_DEF_MessageType_tag2el[] = {
    { (ASN_TAG_CLASS_APPLICATION | (1 << 2)), 0, 0, 0 }, /* undirectional at 19 */
    { (ASN_TAG_CLASS_APPLICATION | (2 << 2)), 1, 0, 0 }, /* begin at 20 */
    { (ASN_TAG_CLASS_APPLICATION | (4 << 2)), 2, 0, 0 }, /* end at 21 */
    { (ASN_TAG_CLASS_APPLICATION | (5 << 2)), 3, 0, 0 }, /* contiinue at 22 */
    { (ASN_TAG_CLASS_APPLICATION | (7 << 2)), 4, 0, 0 }, /* abort at 24 */
};
static asn_CHOICE_specifics_t asn_DEF_MessageType_specs = {
  sizeof(struct MessageType),
  offsetof(struct MessageType, _asn_ctx),
  offsetof(struct MessageType, present),
  sizeof(((struct MessageType *)0)->present),
  asn_DEF_MessageType_tag2el,
  5,  /* Count of tags in the map */
  0 /* Whether extensible */
};
asn_TYPE_descriptor_t asn_DEF_MessageType = {
  "MessageType",
  "MessageType",
  CHOICE_free,
  CHOICE_print,
  CHOICE_constraint,
  CHOICE_decode_ber,
  CHOICE_encode_der,
  CHOICE_decode_xer,
  CHOICE_encode_xer,
  CHOICE_outmost_tag,
  0,  /* No effective tags (pointer) */
  0,  /* No effective tags (count) */
  0,  /* No tags (pointer) */
  0,  /* No tags (count) */
  asn_MBR_MessageType,
  5,  /* Elements count */
  &asn_DEF_MessageType_specs  /* Additional specs */
};
