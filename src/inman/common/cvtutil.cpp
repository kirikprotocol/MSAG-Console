/* This module contains various text conversion functions widely used in 3GPP */
#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace cvtutil {

const char k_TBCD_alphabet[15] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'a', 'b', 'c'
};

unsigned char  const k_cvt_7bit_2_8bit[128] = {
/*  0*/ 0x40,0xa3,0x24,0xa5,0xe8,0xe9,0xf9,0xec,
/*  8*/ 0xf2,0xc7,0x0a,0xd8,0xf8,0x0d,0xc5,0xe5,
/* 16*/ 0x44,0x5f,0x46,0x47,0x4c,0x57,0x50,0x59,
/* 24*/ 0x53,0x51,0x58,0x00,0xc6,0xe6,0xdf,0xc9,
/* 32*/ 0x20,0x21,0x22,0x23,0xa4,0x25,0x26,0x27,
/* 40*/ 0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
/* 48*/ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
/* 56*/ 0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
/* 64*/ 0xa1,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
/* 72*/ 0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
/* 80*/ 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
/* 88*/ 0x58,0x59,0x5a,0xc4,0xd6,0xd1,0xdc,0xa7,
/* 96*/ 0xbf,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
/*104*/ 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
/*112*/ 0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
/*120*/ 0x78,0x79,0x7a,0xe4,0xf6,0xf1,0xfc,0xe0
};

unsigned char  const k_cvt_8bit_2_7bit[256] = {
/*  0*/ 0x1b,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/*  8*/ 0x54,0x54,0x0a,0x54,0x54,0x0d,0x54,0x54,
/* 16*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/* 24*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/* 32*/ 0x20,0x21,0x22,0x23,0x02,0x25,0x26,0x27,
/* 40*/ 0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
/* 48*/ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
/* 56*/ 0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
/* 64*/ 0x00,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
/* 72*/ 0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
/* 80*/ 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
/* 88*/ 0x58,0x59,0x5a,0x54,0x54,0x54,0x54,0x11,
/* 96*/ 0x54,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
/*104*/ 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
/*112*/ 0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
/*120*/ 0x78,0x79,0x7a,0x54,0x54,0x54,0x54,0x54,
/*128*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/*136*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/*144*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/*152*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/*160*/ 0x54,0x40,0x54,0x01,0x24,0x03,0x54,0x5f,
/*168*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/*176*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
/*184*/ 0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x60,
/*192*/ 0x54,0x54,0x54,0x54,0x5b,0x0e,0x1c,0x09,
/*200*/ 0x54,0x1f,0x54,0x54,0x54,0x54,0x54,0x54,
/*208*/ 0x54,0x5d,0x54,0x54,0x54,0x54,0x5c,0x54,
/*216*/ 0x0b,0x54,0x54,0x54,0x5e,0x54,0x54,0x1e,
/*224*/ 0x7f,0x54,0x54,0x54,0x7b,0x0f,0x1d,0x54,
/*232*/ 0x04,0x05,0x54,0x54,0x07,0x54,0x54,0x54,
/*240*/ 0x54,0x7d,0x08,0x54,0x54,0x54,0x7c,0x54,
/*248*/ 0x0c,0x06,0x54,0x54,0x7e,0x54,0x54,0x54
};

/* ************************************************************************** *
 * Various text and alphabets conversion utility functions
 * ************************************************************************** */
/* Packs NumericString ('0'-'9') as Binary Coded Decimal String.
 * By default, in case of odd string length, the '1111'B filler is added
 * to high semioctet of last byte.
 *
 * NOTE: function doesn't perform check for output buffer ABW ('Array out
 * of Boundary Writing', its size is precisely determined, so
 * it's caller responsibility to provide necessary space.
 */
unsigned short packNumString2BCD(unsigned char * bcd_buf, const char * in_str,
                                 unsigned short str_len, const bool filler/* = true*/)
{
  unsigned short bcdLen = (str_len + 1)/2;

  for (unsigned short i = 0; i < str_len; ++i) {
    if (i & 0x01) // i % 2
      bcd_buf[i/2] |= ((in_str[i] - '0') << 4);  // fill high semioctet
    else
      bcd_buf[i/2] = (in_str[i] - '0') & 0x0F;   // fill low semioctet
  }
  if ((str_len % 2) && filler)
    bcd_buf[bcdLen - 1] |= 0xF0;  // add filler to high semioctet

  return bcdLen;
}

// Unpacks BCD coded numeric string up to 64K length.
// Returns signed number of digits unpacked:
// positive if succeeded, negative if illegal symbol was met.
// NOTE: all illegal symbols is replaced by 'ill_char'
int unpackBCD2NumString(const unsigned char * bcd_buf, char * out_str,
                        unsigned short bcd_len, const char ill_char/* = 0*/)
{
  bool            illegal = false;
  unsigned short  i = 0, k = 0;

  for (; i < bcd_len; ++i, ++k) {
    // low semioctet
    if ((out_str[k] = bcd2char(bcd_buf[i], ill_char)) == ill_char)
      illegal = true;
    // high semioctet, check for possible filler 
    if ((out_str[++k] = bcd2char(bcd_buf[i] >> 4, ill_char)) == ill_char) {
      if ((bcd_buf[i] & 0xF0) == 0xF0) //filler
        break;
      else
        illegal = true;
    }
  }
  out_str[k] = 0;
  return illegal ? -(int)k : (int)k;
}

// Unpacks TBCD (Telephony BCD) coded string up to 64K length.
// Returns signed number of digits unpacked:
// positive if succeeded, negative if illegal symbol was met.
// NOTE: all illegal symbols is replaced by 'ill_char'
int unpackTBCD2String(const unsigned char * bcd_buf, char * out_str,
                      unsigned short bcd_len, const char ill_char/* = 0*/)
{
  bool            illegal = false;
  unsigned short  i = 0, k = 0;

  for (; i < bcd_len; ++i, ++k) {
    // low semioctet
    if ((out_str[k] = tbcd2char(bcd_buf[i], ill_char)) == ill_char)
      illegal = true;
    // high semioctet, check for possible filler 
    if ((out_str[++k] = tbcd2char(bcd_buf[i] >> 4, ill_char)) == ill_char) {
      if ((bcd_buf[i] & 0xF0) == 0xF0) //filler
        break;
      else
        illegal = true;
    }
  }
  out_str[k] = 0;
  return illegal ? -(int)k : (int)k;
}


// Adds symbol to buffer containing packed 7bit chars
void packCharAs7Bit(unsigned char* & ptr, unsigned & shift, unsigned char val8bit)
{
  *ptr = (*ptr & (0xFF >> (8 - shift))) | (val8bit << shift);
  if (shift > 1)
    *(ptr + 1) = (*(ptr + 1) & (0xFF << shift)) | (val8bit >> (8 - shift));
  if ((shift += 7) >= 8) {
    shift &= 0x7;
    ++ptr;
  }
}

// Extracts 7Bit character 'as is' from buffer containing packed 7Bit text.
unsigned char get7BitChar(unsigned char* & ptr, unsigned & shift)
{
  char val = (*ptr >> shift) & 0x7f;

  if (shift > 1)
    val |= (*(ptr + 1) << (8 - shift)) & 0x7f;
  if ((shift += 7) >= 8) {
    shift &= 0x7;
    ++ptr;
  }
  return val;
}


/* macro checking for A(rray)B(ounds)R(ead)attemt while		*
 * extracting 7bit char from buffer containing packed 7bit text */
#define __notABR7(p, pEnd, ofs)  ((p + (ofs > 1 ? 1 : 0)) < pEnd)

// Determines number of packed 7bit chars in buffer keeping packed 7bit text, 
// which are unpacked to single text symbol.
// Returns 1 or 2, zero in case of end of array.
unsigned char estimate7BitChar(unsigned char* & b7ptr,
                               const unsigned char* & b7End, unsigned & shift)
{
  unsigned char num7ch = 0;

  if (__notABR7(b7ptr, b7End, shift)) {
    unsigned char ch = k_cvt_7bit_2_8bit[get7BitChar(b7ptr, shift)];
    ++num7ch;
    /* check for escaped character and don't forget about ABR check! */
    if ((ch == 0x1b) && __notABR7(b7ptr, b7End, shift))
      ++num7ch;
  }
  return num7ch;
}


// Determines the length (in bytes) of packed 7bit text converted from input text.
// If 'p_7bit_cnt' is non NULL, number of 7bit chars in output buffer is returned.
unsigned estimateTextAs7Bit(const char* text, unsigned tlen, unsigned * p_7bit_cnt)
{
  unsigned  shift = 0, num7ch = 0, numBytes = tlen ? 1 : 0;

  for (unsigned i = 0; i < tlen; ++i) {
#define __pchar()   if ((shift += 7) >= 8) { shift &= 0x7; ++numBytes; } ++num7ch
#define __escape()  __pchar(); __pchar();
    switch (text[i]) {
    case '^':   __escape(); break;
    case '\f':  __escape(); break;
    case '|':   __escape(); break;
    case '{':   __escape(); break;
    case '}':   __escape(); break;
    case '[':   __escape(); break;
    case ']':   __escape(); break;
    case '~':   __escape(); break;
    case '\\':  __escape(); break;
    default:    __pchar();
    }
#undef __pchar
#undef __escape
  }
  if (p_7bit_cnt)
    *p_7bit_cnt = num7ch;
  numBytes += (shift ? 1 : 0);
  return numBytes;
}

// Determines the length of resulted Latin1 text string unpacked from given
// buffer, containing packed 7bit text. 
// If 'p_7bit_cnt' is non NULL, number of 7bit chars in input buffer is returned.
//
// NOTE: returned length doesn't take into account terminating zero.
unsigned estimate7BitAsText(const unsigned char* b7_buf, unsigned b7_len, unsigned * p_7bit_cnt)
{
  unsigned  ch, tlen = 0, shift = 0,
            num7ch = p_7bit_cnt ? *p_7bit_cnt : 0;
  unsigned char *ptr  = (unsigned char*)b7_buf;
  const unsigned char *ptrEnd = b7_buf + b7_len;

  while ((ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch)))
    tlen++;

  if (p_7bit_cnt)
    *p_7bit_cnt = num7ch;
  return tlen;
}

/* Converts Latin1 text to GSM 7 bit Default Alphabet and packs it as specified in
 * 3GPP TS 23.038 [Character Packing]
 *
 * NOTE: because of input text may contain symbols from GSM 7 bit default alphabet
 * extension table, which are escaped, it's impossible to precisely determine length
 * of output buffer, so caller should estimate necessary buffer size and pass it to
 * function in order to perform ABW check.
 *
 * NOTE: this function doesn't suitable for consequetive packing.
 */
unsigned packTextAs7BitSafe(const char* text, unsigned tlen, unsigned char* b7_buf,
                            unsigned b7_len, unsigned * p_7bit_cnt)
{
  unsigned char*  base = b7_buf;
  unsigned char*  b7buf_end = base + b7_len;
  unsigned        shift = 0, num7ch = 0;

  for (unsigned i = 0; i < tlen; ++i) {
#define __pchar(x)	packCharAs7BitSafe(b7_buf, shift, x, b7buf_end); ++num7ch
#define __escape(x)	__pchar(0x1b); __pchar(x);
    switch (text[i]) {
    case '^': __escape(0x14); break;
    case '\f':__escape(0x0a); break;
    case '|': __escape(0x40); break;
    case '{': __escape(0x28); break;
    case '}': __escape(0x29); break;
    case '[': __escape(0x3c); break;
    case ']': __escape(0x3e); break;
    case '~': __escape(0x3d); break;
    case '\\':__escape(0x2f); break;
    default:
      __pchar(k_cvt_8bit_2_7bit[(uint8_t)text[i]]);
    }
#undef __pchar
#undef __escape
  }
  if (p_7bit_cnt)
    *p_7bit_cnt = num7ch;
  return ((unsigned)(b7_buf - base) + (shift ? 1 : 0));
}

unsigned packTextAs7Bit(const char* text, unsigned tlen, unsigned char* b7_buf,
                        unsigned * p_7bit_cnt)
{
  unsigned char*  base = b7_buf;
  unsigned        shift = 0, num7ch = 0;

  for (unsigned i = 0; i < tlen; ++i) {
#define __pchar(x)	packCharAs7Bit(b7_buf, shift, x); ++num7ch
#define __escape(x)	__pchar(0x1b); __pchar(x);
    switch (text[i]) {
    case '^': __escape(0x14); break;
    case '\f':__escape(0x0a); break;
    case '|': __escape(0x40); break;
    case '{': __escape(0x28); break;
    case '}': __escape(0x29); break;
    case '[': __escape(0x3c); break;
    case ']': __escape(0x3e); break;
    case '~': __escape(0x3d); break;
    case '\\':__escape(0x2f); break;
    default:
      __pchar(k_cvt_8bit_2_7bit[(uint8_t)text[i]]);
    }
#undef __pchar
#undef __escape
  }
  if (p_7bit_cnt)
    *p_7bit_cnt = num7ch;
  return ((unsigned)(b7_buf - base) + (shift ? 1 : 0));
}


unsigned packTextAs7BitPaddedSafe(const char* text, unsigned tlen,
                                  unsigned char* octs, unsigned olen)
{
  unsigned  num7ch = 0;
  unsigned  packedLen = packTextAs7BitSafe(text, tlen, octs, olen, &num7ch);

  if ((packedLen*8 - num7ch*7) == 7)
    /* last octet has 7 zero bits, fill them with CR, in order to distinguish with '@' */
    octs[packedLen - 1] = (0x0d << 1) | (octs[packedLen - 1] & 0x01);

  return packedLen;
}


unsigned packTextAs7BitPadded(const char* text, unsigned tlen, unsigned char* octs)
{
  unsigned  num7ch = 0;
  unsigned  packedLen = packTextAs7Bit(text, tlen, octs, &num7ch);

  if ((packedLen*8 - num7ch*7) == 7)
    /* last octet has 7 zero bits, fill them with CR, in order to distinguish with '@' */
    octs[packedLen - 1] = (0x0d << 1) | (octs[packedLen - 1] & 0x01);

  return packedLen;
}

// Unpacks and converts 7Bit character from GSM 7 bit Default Alphabet to text symbol.
// Returns 0 on end of array.
unsigned char unpack7BitChar(unsigned char*& b7ptr, const unsigned char*& b7End,
                             unsigned& shift, unsigned& num7ch)
{
  unsigned char ch = 0;

  if (__notABR7(b7ptr, b7End, shift)) {
    ch = k_cvt_7bit_2_8bit[get7BitChar(b7ptr, shift)];
    num7ch++;
    /* check for escaped character and don't forget about ABR check! */
    if ((ch == 0x1b)) {
      if (__notABR7(b7ptr, b7End, shift)) {
        switch (ch = k_cvt_7bit_2_8bit[get7BitChar(b7ptr, shift)]) {
        case 0x14: ch = '^' ; break;
        case 0x0a: ch = '\f'; break;
        case 0x40: ch = '|' ; break;
        case 0x28: ch = '{' ; break;
        case 0x29: ch = '}' ; break;
        case 0x3c: ch = '[' ; break;
        case 0x3e: ch = ']' ; break;
        case 0x3d: ch = '~' ; break;
        case 0x2f: ch = '\\'; break;
        default:;
        }
        num7ch++;
      } else /* 1b is last char (broken 7bit buffer), convert it to space */
        ch = ' ';
    }
  }
  return ch;
}


/* Unpacks and converts packed 7Bit text from GSM 7 bit Default Alphabet to
 * zero-terminated Latin1 text string.
 * Returns number of characters in output string (zero doesn't accounted).
 */
unsigned unpack7Bit2Text(const unsigned char* b7_buf, unsigned b7_len,
                         unsigned char* text, unsigned * p_7bit_cnt)
{
  unsigned  tlen = 0, shift = 0, num7ch = 0;
  unsigned char	ch, *ptr  = (unsigned char*)b7_buf;
  const unsigned char *ptrEnd = b7_buf + b7_len;

  while ((ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch)))
    text[tlen++] = ch;
  text[tlen] = 0;

  if (p_7bit_cnt)
    *p_7bit_cnt = num7ch;
  return tlen;
}


unsigned unpack7Bit2Text(const unsigned char* b7_buf, unsigned b7_len,
                            std::string & str, unsigned * p_7bit_cnt)
{
  unsigned  tlen = 0, shift = 0, num7ch = 0;
  unsigned char	ch, *ptr  = (unsigned char*)b7_buf;
  const unsigned char *ptrEnd = b7_buf + b7_len;

  str.clear();
  while ((ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch))) {
    str += ch; tlen++;
  }
  str += '\0';

  if (p_7bit_cnt)
    *p_7bit_cnt = num7ch;
  return tlen;
}


// 'Exception throwing' version of unpack7Bit2Text() that checks for ABW.
unsigned unpack7Bit2TextSafe(const unsigned char* b7_buf, unsigned b7_len,
                             unsigned char* out_str, const unsigned max_tlen,
                             unsigned * p_7bit_cnt)
  throw(std::runtime_error)
{
  unsigned  tlen = 0, shift = 0, num7ch = 0;
  unsigned char ch, *ptr = (unsigned char*)b7_buf;
  const unsigned char *ptrEnd = b7_buf + b7_len;

  while ((ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch))) {
    if (tlen < max_tlen)
      out_str[tlen++] = ch;
    else
      throw std::runtime_error("unpack7Bit2Text: ABW attempt detected!");
  }
  out_str[tlen] = 0;

  if (p_7bit_cnt)
    *p_7bit_cnt = num7ch;
  return tlen;
}

/* Unpacks and converts packed 7Bit text from GSM 7 bit Default Alphabet to
 * zero-terminated Latin1 text string. Performs check for 7bit text padding.
 * Returns number of characters in output string (zero doesn't accounted).
 */
unsigned unpack7BitPadded2Text(const unsigned char* b7_buf, unsigned b7_len,
                               unsigned char* out_str)
{
  unsigned  num7ch = 0;
  unsigned  tlen = unpack7Bit2Text(b7_buf, b7_len, out_str, &num7ch);

  if ((out_str[tlen - 1] == '\r') && (b7_len*8 == num7ch*7))
  /* 7bit text was padded with CR in order to distinguish last 7 zero bits with '@' */
    out_str[--tlen] = 0;
  return tlen;
}

unsigned unpack7BitPadded2Text(const unsigned char* b7_buf, unsigned b7_len,
                                                        std::string & out_str)
{
  unsigned  num7ch = 0;
  unsigned  tlen = unpack7Bit2Text(b7_buf, b7_len, out_str, &num7ch);
  
  if ((out_str[tlen - 1] == '\r') && (b7_len*8 == num7ch*7))
  /* 7bit text was padded with CR in order to distinguish last 7 zero bits with '@' */
    out_str[--tlen] = 0;
  return tlen;
}

unsigned unpack7BitPadded2TextSafe(const unsigned char* b7_buf, unsigned b7_len,
                                   unsigned char* out_str, const unsigned max_tlen)
  throw(std::runtime_error)
{
  unsigned  num7ch = 0;
  unsigned  tlen = unpack7Bit2TextSafe(b7_buf, b7_len, out_str, max_tlen, &num7ch);

  if ((out_str[tlen - 1] == '\r') && (b7_len*8 == num7ch*7))
  /* 7bit text was padded with CR in order to distinguish last 7 zero bits with '@' */
    out_str[--tlen] = 0;
  return tlen;
}

} //cvtutil
} //smsc

