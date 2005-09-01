#include <asn_internal.h>

#include <Component.h>

static asn_TYPE_member_t asn_MBR_Component[] = {
  { ATF_NOFLAGS, 0, offsetof(struct Component, choice.invoke),
    (ASN_TAG_CLASS_CONTEXT | (1 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_Invoke,
    0,  /* Defer constraints checking to the member type */
    "invoke"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Component, choice.returnResultLast),
    (ASN_TAG_CLASS_CONTEXT | (2 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_ReturnResult,
    0,  /* Defer constraints checking to the member type */
    "returnResultLast"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Component, choice.returnError),
    (ASN_TAG_CLASS_CONTEXT | (3 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_ReturnError,
    0,  /* Defer constraints checking to the member type */
    "returnError"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Component, choice.reject),
    (ASN_TAG_CLASS_CONTEXT | (4 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_Reject,
    0,  /* Defer constraints checking to the member type */
    "reject"
    },
  { ATF_NOFLAGS, 0, offsetof(struct Component, choice.returnResultNotLast),
    (ASN_TAG_CLASS_CONTEXT | (7 << 2)),
    -1, /* IMPLICIT tag at current level */
    (void *)&asn_DEF_ReturnResult,
    0,  /* Defer constraints checking to the member type */
    "returnResultNotLast"
    },
};
static asn_TYPE_tag2member_t asn_DEF_Component_tag2el[] = {
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 }, /* invoke at 101 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 }, /* returnResultLast at 102 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }, /* returnError at 103 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* reject at 104 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 4, 0, 0 }, /* returnResultNotLast at 106 */
};
static asn_CHOICE_specifics_t asn_DEF_Component_specs = {
  sizeof(struct Component),
  offsetof(struct Component, _asn_ctx),
  offsetof(struct Component, present),
  sizeof(((struct Component *)0)->present),
  asn_DEF_Component_tag2el,
  5,  /* Count of tags in the map */
  0 /* Whether extensible */
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
  CHOICE_outmost_tag,
  0,  /* No effective tags (pointer) */
  0,  /* No effective tags (count) */
  0,  /* No tags (pointer) */
  0,  /* No tags (count) */
  asn_MBR_Component,
  5,  /* Elements count */
  &asn_DEF_Component_specs  /* Additional specs */
};
