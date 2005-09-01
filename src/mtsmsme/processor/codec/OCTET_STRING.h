#ifndef _OCTET_STRING_H_
#define _OCTET_STRING_H_

#include <asn_application.h>

typedef struct OCTET_STRING {
  uint8_t *buf; /* Buffer with consecutive OCTET_STRING bits */
  int size; /* Size of the buffer */

  asn_struct_ctx_t _asn_ctx;  /* Parsing across buffer boundaries */
} OCTET_STRING_t;

extern asn_TYPE_descriptor_t asn_DEF_OCTET_STRING;

asn_struct_free_f OCTET_STRING_free;
asn_struct_print_f OCTET_STRING_print;
asn_struct_print_f OCTET_STRING_print_utf8;
ber_type_decoder_f OCTET_STRING_decode_ber;
der_type_encoder_f OCTET_STRING_encode_der;
xer_type_decoder_f OCTET_STRING_decode_xer_hex;   /* Hexadecimal */
xer_type_decoder_f OCTET_STRING_decode_xer_binary;  /* 01010111010 */
xer_type_decoder_f OCTET_STRING_decode_xer_utf8;  /* ASCII/UTF-8 */
xer_type_encoder_f OCTET_STRING_encode_xer;
xer_type_encoder_f OCTET_STRING_encode_xer_utf8;

/******************************
 * Handy conversion routines. *
 ******************************/

/*
 * This function clears the previous value of the OCTET STRING (if any)
 * and then allocates a new memory and returns a pointer to it.
 * If size = -1, the size of the original string will be determined
 * using strlen(str).
 * If str equals to NULL, the function will silently clear the
 * current contents of the OCTET STRING.
 * Returns 0 if it was possible to perform operation, -1 otherwise.
 */
int OCTET_STRING_fromBuf(OCTET_STRING_t *s, const char *str, int size);

/* Handy conversion from the C string into the OCTET STRING. */
#define OCTET_STRING_fromString(s, str) OCTET_STRING_fromBuf(s, str, -1);

/*
 * Allocate and fill the new OCTET STRING and return a pointer to the newly
 * allocated object. NULL is permitted in str: the function will just allocate
 * empty OCTET STRING.
 */
OCTET_STRING_t *OCTET_STRING_new_fromBuf(asn_TYPE_descriptor_t *td,
  const char *str, int size);

/****************************
 * Internally useful stuff. *
 ****************************/

typedef struct asn_OCTET_STRING_specifics_s {
  /*
   * Target structure description.
   */
  int struct_size;  /* Size of the structure */
  int ctx_offset;   /* Offset of the asn_struct_ctx_t member */

  int subvariant;   /* {0,1,2} for O-S, BIT STRING or ANY */
} asn_OCTET_STRING_specifics_t;

#endif  /* _OCTET_STRING_H_ */
