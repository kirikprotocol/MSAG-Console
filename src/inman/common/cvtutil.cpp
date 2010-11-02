/* This module contains various conversion functions widely used in 3GPP */
#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace cvtutil {

unsigned char  const _cvt_7bit_2_8bit[128] = {
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

unsigned char  const _cvt_8bit_2_7bit[256] = {
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

/* GVR:
 * Determines the length (in bytes) of packed 7bit text converted from input text.
 * If '_7bit_chars' is non NULL, number of 7bit chars in output buffer is returned.
 */
unsigned estimateTextAs7Bit(const char* text, unsigned tlen, unsigned * _7bit_chars)
{
    unsigned	shift = 0, num7ch = 0, numBytes = tlen ? 1 : 0;

    for (unsigned i = 0; i < tlen; ++i) {
#define __pchar()	if ((shift += 7) >= 8) { shift &= 0x7; ++numBytes; } ++num7ch
#define __escape()	__pchar(); __pchar();
	switch (text[i]) {
	case '^': __escape(); break;
	case '\f':__escape(); break;
	case '|': __escape(); break;
	case '{': __escape(); break;
	case '}': __escape(); break;
	case '[': __escape(); break;
	case ']': __escape(); break;
	case '~': __escape(); break;
	case '\\':__escape(); break;
	default:
	    __pchar();
	}
#undef __pchar
#undef __escape
    }
    if (_7bit_chars)
	*_7bit_chars = num7ch;
    numBytes += (shift ? 1 : 0);
    return numBytes;

}


/* GVR:
 * Determines the length of resulted text unpacked from given buffer containing
 * packed 7bit text. If '_7bit_chars' is non NULL, number of 7bit chars in input
 * buffer is returned.
 *
 * NOTE: returned length doesn't take into account terminating zero.
 */
unsigned estimate7BitAsText(const unsigned char* b7buf, unsigned b7len, unsigned * _7bit_chars)
{
    unsigned		ch, tlen = 0, shift = 0,
			num7ch = _7bit_chars ? *_7bit_chars : 0;
    unsigned char	*ptr  = (unsigned char*)b7buf;
    const unsigned char *ptrEnd = b7buf + b7len;

    while (ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch))
	tlen++;

    if (_7bit_chars)
	*_7bit_chars = num7ch;
    return tlen;
}

/* GVR:
 * Converts text to GSM 7 bit Default Alphabet and packs it as specified in
 * 3GPP TS 23.038 [Character Packing]
 *
 * NOTE: because of input text may contain symbols from GSM 7 bit default alphabet
 * extension table, which are escaped, it's impossible to precisely determine length
 * of output buffer, so caller should estimate necessary buffer size and pass it to
 * function in order to perform ABW check.
 *
 * NOTE: this function doesn't suitable for consequetive packing, see ConvertText27bit()
 */
unsigned packTextAs7BitSafe(const char* text, unsigned tlen, unsigned char* b7buf,
						unsigned b7len, unsigned * _7bit_chars)
{
    unsigned char*  base = b7buf;
    unsigned char*  b7buf_end = base + b7len;
    unsigned	    shift = 0, num7ch = 0;

    for (unsigned i = 0; i < tlen; ++i) {
#define __pchar(x)	packCharAs7BitSafe(b7buf, shift, x, b7buf_end); ++num7ch
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
	    __pchar(_cvt_8bit_2_7bit[(uint8_t)text[i]]);
	}
#undef __pchar
#undef __escape
    }
    if (_7bit_chars)
	*_7bit_chars = num7ch;
    return ((unsigned)(b7buf - base) + (shift ? 1 : 0));
}

unsigned packTextAs7Bit(const char* text, unsigned tlen, unsigned char* b7buf,
								unsigned * _7bit_chars)
{
    unsigned char*  base = b7buf;
    unsigned	    shift = 0, num7ch = 0;

    for (unsigned i = 0; i < tlen; ++i) {
#define __pchar(x)	packCharAs7Bit(b7buf, shift, x); ++num7ch
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
	    __pchar(_cvt_8bit_2_7bit[(uint8_t)text[i]]);
	}
#undef __pchar
#undef __escape
    }
    if (_7bit_chars)
	*_7bit_chars = num7ch;
    return ((unsigned)(b7buf - base) + (shift ? 1 : 0));
}


unsigned packTextAs7BitPaddedSafe(const char* text, unsigned tlen,
				    unsigned char* octs, unsigned olen)
{
    unsigned	num7ch = 0;
    unsigned	packedLen = packTextAs7BitSafe(text, tlen, octs, olen, &num7ch);

    if (packedLen*8 - num7ch*7 == 7 )
    /* last octet has 7 zero bits, fill them with CR, in order to distinguish with '@' */
	octs[packedLen - 1] = (0x0d << 1) | (octs[packedLen - 1] & 0x01);

    return packedLen;
}


unsigned packTextAs7BitPadded(const char* text, unsigned tlen, unsigned char* octs)
{
    unsigned	num7ch = 0;
    unsigned	packedLen = packTextAs7Bit(text, tlen, octs, &num7ch);

    if (packedLen*8 - num7ch*7 == 7)
    /* last octet has 7 zero bits, fill them with CR, in order to distinguish with '@' */
	octs[packedLen - 1] = (0x0d << 1) | (octs[packedLen - 1] & 0x01);

    return packedLen;
}

/* GVR:
 * Unpacks and converts 7Bit character from GSM 7 bit Default Alphabet to text symbol.
 * Returns 0 on end of array.
 */
unsigned char unpack7BitChar(unsigned char*& b7ptr, const unsigned char*& b7End,
					unsigned& shift, unsigned& num7ch)
{
    unsigned char ch = 0;

    if (__notABR7(b7ptr, b7End, shift)) {
	ch = _cvt_7bit_2_8bit[get7BitChar(b7ptr, shift)];
	num7ch++;
	/* check for escaped character and don't forget about ABR check! */
	if ((ch == 0x1b)) {
	    if (__notABR7(b7ptr, b7End, shift)) {
		switch (ch = _cvt_7bit_2_8bit[get7BitChar(b7ptr, shift)]) {
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


/* GVR:
 * Unpacks and converts packed 7Bit text from GSM 7 bit Default Alphabet to ASCII text.
 * Returns number of characters (terminating zero doesn't accounted).
 */
unsigned unpack7Bit2Text(const unsigned char* b7buf, unsigned b7len,
				unsigned char* text, unsigned * _7bit_chars)
{
    unsigned		tlen = 0, shift = 0, num7ch = 0;
    unsigned char	ch, *ptr  = (unsigned char*)b7buf;
    const unsigned char *ptrEnd = b7buf + b7len;

    while (ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch))
	text[tlen++] = ch;
    text[tlen] = 0;

    if (_7bit_chars)
	*_7bit_chars = num7ch;
    return tlen;
}


unsigned unpack7Bit2Text(const unsigned char* b7buf, unsigned b7len,
                            std::string & str, unsigned * _7bit_chars)
{
    unsigned		tlen = 0, shift = 0, num7ch = 0;
    unsigned char	ch, *ptr  = (unsigned char*)b7buf;
    const unsigned char *ptrEnd = b7buf + b7len;

    str.clear();
    while (ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch)) {
        str += ch; tlen++;
    }
    str += '\0';

    if (_7bit_chars)
	*_7bit_chars = num7ch;
    return tlen;
}


/* GVR:
 * 'Exception throwing' version of unpack7Bit2Text() that checks for ABW.
 */
unsigned unpack7Bit2TextSafe(const unsigned char* b7buf, unsigned b7len,
				unsigned char* text, unsigned maxtlen,
				unsigned * _7bit_chars) throw(std::runtime_error)
{
    unsigned		tlen = 0, shift = 0, num7ch = 0;
    unsigned char	ch, *ptr  = (unsigned char*)b7buf;
    const unsigned char *ptrEnd = b7buf + b7len;

    while (ch = unpack7BitChar(ptr, ptrEnd, shift, num7ch)) {
	if (tlen < maxtlen)
	    text[tlen++] = ch;
	else
	    throw std::runtime_error("unpack7Bit2Text: ABW attempt detected!");
    }
    text[tlen] = 0;

    if (_7bit_chars)
	*_7bit_chars = num7ch;
    return tlen;
}

/* GVR:
 * Unpacks and converts packed 7Bit text from GSM 7 bit Default Alphabet to ASCII text.
 * performing check for 7bit text padding.
 * Returns number of characters (terminating zero doesn't accounted).
 */
unsigned unpack7BitPadded2Text(const unsigned char* b7buf, unsigned b7len,
							unsigned char* text)
{
    unsigned	num7ch = 0;
    unsigned	tlen = unpack7Bit2Text(b7buf, b7len, text, &num7ch);

    if ((text[tlen] == '\r') && (b7len*8 - num7ch*7 == 7))

    /* 7bit text was padded with CR in order to distinguish last 7 zero bits with '@' */
	text[tlen--] = 0;
    return tlen;
}
unsigned unpack7BitPadded2Text(const unsigned char* b7buf, unsigned b7len,
                                                        std::string & str)
{
    unsigned	num7ch = 0;
    unsigned	tlen = unpack7Bit2Text(b7buf, b7len, str, &num7ch);

    if ((str[tlen] == '\r') && (b7len*8 - num7ch*7 == 7))

    /* 7bit text was padded with CR in order to distinguish last 7 zero bits with '@' */
	str[tlen--] = 0;
    return tlen;
}


unsigned unpack7BitPadded2TextSafe(const unsigned char* b7buf, unsigned b7len,
						unsigned char* text, unsigned maxtlen)
                                    throw(std::runtime_error)
{
    unsigned	num7ch = 0;
    unsigned	tlen = unpack7Bit2TextSafe(b7buf, b7len, text, maxtlen, &num7ch);

    if ((text[tlen] == '\r') && (b7len*8 - num7ch*7 == 7))

    /* 7bit text was padded with CR in order to distinguish last 7 zero bits with '@' */
	text[tlen--] = 0;
    return tlen;
}


/* ************************************************************************** *
 * Time conversion utility functions
 * ************************************************************************** */
#ifndef _REENTRANT
#define _REENTRANT	//turn on localtime_r() definition
#endif /* _REENTRANT */
#include <time.h>


/*
 * Unpacks time stored in BCD7 format (YYMMDDHHmmssZZ) to structure
 * holding date/time fields that is local for timezone identified by 'qtz'
 */
int unpack7BCD2TimeSTZ(unsigned char (*bcd)[7], struct tm &tms, int &qtz)
{
    unsigned tzOctet = unpackBCDOct2Num((*bcd)[6]);

    tms.tm_isdst = -1; /* let mktime() to determine wether the DST is appliable */
    tms.tm_wday = tms.tm_yday = 0;
    tms.tm_year = unpackBCDOct2Num((*bcd[0]));
    tms.tm_mon  = unpackBCDOct2Num((*bcd[1]));
    tms.tm_mday = unpackBCDOct2Num((*bcd[2]));
    tms.tm_hour = unpackBCDOct2Num((*bcd[3]));
    tms.tm_min  = unpackBCDOct2Num((*bcd[4]));
    tms.tm_sec  = unpackBCDOct2Num((*bcd[5]));
    qtz = !(tzOctet & 0x80) ? (int)tzOctet : (-(int)(tzOctet&0x07f));

    if (!(tms.tm_year >= 0 && tms.tm_year <= 99))
	return _SMSC_CVT_BAD_YEAR;
    tms.tm_year += 100; // year = x-1900
    
    if (!( tms.tm_mon >= 1 && tms.tm_mon <= 12))
	return _SMSC_CVT_BAD_YEAR;
    tms.tm_mon -= 1;	// adjust month to [0..11]

    if (!(tms.tm_mday >= 1 && tms.tm_mday <= 31))
	return _SMSC_CVT_BAD_DAY;
    if (!(tms.tm_hour >= 0 && tms.tm_hour <= 23))
	return _SMSC_CVT_BAD_HOUR;
    if (!(tms.tm_min >= 0 && tms.tm_min <= 59))
	return _SMSC_CVT_BAD_MIN;
    if (!(tms.tm_sec >=0 && tms.tm_sec <= 59))
	return _SMSC_CVT_BAD_SEC;
    if (!(qtz >= -47 && qtz <= 48))
	return  _SMSC_CVT_BAD_QTZ;
    return 0;
}

//GMT = LocalTime + TZSeconds; TZSeconds = (-QTZ)*15*60
time_t cvtTimeSTZ2UTCTimeT(struct tm &tms, int &qtz)
{
/*  NOTE: mktime() adjust time_t for local timezone and DST */
    time_t tmVal = mktime(&tms);

    if (tmVal != (-1)) {
    /* consider the 'qtz' differs from local TZ, adjust time_t for difference */
	tmVal += (long)qtz*15*60 - timezone;
    }
    return tmVal;
}


time_t unpack7BCD2UTCTimeT(unsigned char (*bcd)[7])
{
    int 	gtz;
    struct tm	tms;

    if (unpack7BCD2TimeSTZ(bcd, tms, gtz))
	return (time_t)(-1);

    return cvtTimeSTZ2UTCTimeT(tms, gtz);
}


void packTimeSTZ2BCD7(unsigned char (*bcd)[7], struct tm &tms, int &qtz)
{
    unsigned unum = (tms.tm_year > 99) ? (tms.tm_year - 100) : tms.tm_year;

    (*bcd)[0] = packTinyNum2BCDOct(unum);
    unum = tms.tm_mon + 1; //adjust to [1..12]
    (*bcd)[1] = packTinyNum2BCDOct(unum);
    (*bcd)[2] = packTinyNum2BCDOct((unsigned)tms.tm_mday);
    (*bcd)[3] = packTinyNum2BCDOct((unsigned)tms.tm_hour);
    (*bcd)[4] = packTinyNum2BCDOct((unsigned)tms.tm_min);
    (*bcd)[5] = packTinyNum2BCDOct((unsigned)tms.tm_sec);
    unum = (qtz < 0) ? 0x80 | (unsigned char)(-qtz): qtz;
    (*bcd)[6] = packTinyNum2BCDOct(unum);
}

void packTimeSTZ2BCD8(unsigned char (*bcd)[8], struct tm &tms, int &qtz)
{
    /* pack millenium & century */
    unsigned unum = (tms.tm_year + 1900)/100;
    (*bcd)[0] = packTinyNum2BCDOct(unum);
    /* pack other parts */
    packTimeSTZ2BCD7((unsigned char (*)[7])(&((*bcd)[1])), tms, qtz);
}


/* TZ for Nsk is GMT-6, timezone is -21600 => gtz is 24 */
#define cvtUTCTimeT2TimeSTZ(tmVal, ltms, qtz) \
{   if (!localtime_r(tmVal, &ltms)) return _SMSC_CVT_BAD_TIME; \
    qtz = (int)(-timezone/(15*60)); }

int packTimeT2BCD7(unsigned char (*bcd)[7], time_t tmVal)
{
    int		qtz;
    struct tm	ltms;

    cvtUTCTimeT2TimeSTZ(&tmVal, ltms, qtz);
    packTimeSTZ2BCD7(bcd, ltms, qtz);
    return 0;
}

int packTimeT2BCD8(unsigned char (*bcd)[8], time_t tmVal)
{
    int		qtz;
    struct tm	ltms;

    cvtUTCTimeT2TimeSTZ(&tmVal, ltms, qtz);
    packTimeSTZ2BCD8(bcd, ltms, qtz);

/* for debug:
 	(*bcd)[0] = 0x02; (*bcd)[1] = 0x50; 	
 	(*bcd)[2] = 0x90; (*bcd)[3] = 0x51;
 	(*bcd)[4] = 0x71; (*bcd)[5] = 0x92;
 	(*bcd)[6] = 0x00; (*bcd)[7] = 0x00;
*/
    return 0;
}


/* ************************************************************************** *
 * TP Validity Period conversion utility functions
 * ************************************************************************** */

#define MIN5_SECS	300
#define HALF_HOUR_SECS	1800
#define HOUR_SECS	3600
#define HALF_DAY_SECS	(12*3600)
#define DAY_SECS	(24*3600)
#define MONTH_SECS	(30*DAY_SECS)
#define WEEK_SECS	(7*DAY_SECS)
#define WEEKS5_SECS	(35*DAY_SECS)
#define WEEKS63_SECS	(63*WEEK_SECS)

unsigned char packTP_VP_Relative(time_t vpVal)
{
    unsigned char tpVp;

    if (vpVal <= HALF_DAY_SECS) {
	if (vpVal < MIN5_SECS) /* round up to lesser unit */
	    vpVal = MIN5_SECS;
	tpVp = (unsigned char)(vpVal/MIN5_SECS - 1);
    } else if (vpVal <= DAY_SECS) {
	tpVp = (unsigned char)((vpVal - HALF_DAY_SECS)/HALF_HOUR_SECS + 143);
    } else if (vpVal <= MONTH_SECS) {
	if (vpVal < 2*DAY_SECS) /* round up to 2 days*/
	    vpVal = 2*DAY_SECS;
	tpVp = (unsigned char)(vpVal/DAY_SECS + 166);
    } else if (vpVal <= WEEKS63_SECS) {
	if (vpVal < WEEKS5_SECS) /* round up to 5 weeks */
	    vpVal = WEEKS5_SECS;
	tpVp = (unsigned char)(vpVal/WEEK_SECS + 192);
    }
    return tpVp;
}

time_t unpackTP_VP_Relative(unsigned char tpVp)
{
    time_t vpVal;

    if (tpVp <= 143) {		/* [5min .. 12hours], unit is 5 minutes */
	vpVal = (time_t)((unsigned long)tpVp + 1)*MIN5_SECS;
    } else if (tpVp <= 167) {	/* (12hours .. 24hours], unit is 30 minutes */
	vpVal = (time_t)(((unsigned long)tpVp - 143)*HALF_HOUR_SECS + HALF_DAY_SECS);
    } else if (tpVp <= 196) {	/* [2days .. 30days], unit is 1 day */
	vpVal = (time_t)((unsigned long)tpVp - 166)*DAY_SECS;
    } else /* (tpVp <= 255) */ {/* [5weeks .. 63weeks], unit is 1 week */
	vpVal = (time_t)((unsigned long)tpVp - 192)*WEEK_SECS;
    }
    return vpVal;
}

}//namespace cvtutil

/* ************************************************************************** *
 * CBS Data Coding related functions
 * ************************************************************************** */
namespace cbs {
#define BIT_SET(x) (1 << (x))

static ISO_LANG  _langCG0[16] = {
/* 0000 German       */ "DE",
/* 0001 English      */ "EN",
/* 0010 Italian      */ "IT",
/* 0011 French       */ "FR",
/* 0100 Spanish      */ "ES",
/* 0101 Dutch        */ "NL",
/* 0110 Swedish      */ "SV",
/* 0111 Danish       */ "DA",
/* 1000 Portuguese   */ "PT",
/* 1001 Finnish      */ "FI",
/* 1010 Norwegian    */ "NO",
/* 1011 Greek        */ "EL",
/* 1100 Turkish      */ "TR",
/* 1101 Hungarian    */ "HU",
/* 1110 Polish       */ "PL",
/* 1111 unspecified  */ ""
};

static ISO_LANG  _langCG2[16] = {
/* 0000 Czech       */ "CS",
/* 0001 Hebrew      */ "HE",
/* 0010 Arabic      */ "AR",
/* 0011 Russian     */ "RU",
/* 0100 Icelandic   */ "IS",
/* 0101 unspecified */ "",
/* 0110 unspecified */ "",
/* 0111 unspecified */ "",
/* 1000 unspecified */ "",
/* 1001 unspecified */ "",
/* 1010 unspecified */ "",
/* 1011 unspecified */ "",
/* 1100 unspecified */ "",
/* 1101 unspecified */ "",
/* 1110 unspecified */ "",
/* 1111 unspecified */ ""
};

static CBS_DCS::TextEncoding  _enc_enm[4] = {
    CBS_DCS::dcGSM7Bit, CBS_DCS::dcBINARY8, CBS_DCS::dcUCS2, CBS_DCS::dcReserved
};

CBS_DCS::TextEncoding  parseCBS_DCS(uint8_t dcs, CBS_DCS & res)
{
    res.UDHind = res.msgClassDefined = res.compressed = false;
    res.lngPrefix = CBS_DCS::lngNone;
    res.language[0] = res.language[2] = 0;

    uint8_t codingGroup = (dcs >> 4) & 0x0F;
    uint8_t codingScheme = dcs & 0x0F;
  
    switch (codingGroup) {
    case 0x00: {
        res.encoding = CBS_DCS::dcGSM7Bit;
        res.language[0] = _langCG0[codingScheme][0];
        res.language[1] = _langCG0[codingScheme][1];
    } break;
    case 0x01: {
        if (!codingScheme) {
            res.encoding = CBS_DCS::dcGSM7Bit;
            res.lngPrefix = CBS_DCS::lng4GSM7Bit;
        } else if (codingScheme == 1) {
            res.encoding = CBS_DCS::dcUCS2;
            res.lngPrefix = CBS_DCS::lng4UCS2;
        } else 
            res.encoding = CBS_DCS::dcReserved;
    } break;
    case 0x02: {
        res.encoding = CBS_DCS::dcGSM7Bit;
        res.language[0] = _langCG2[codingScheme][0];
        res.language[1] = _langCG2[codingScheme][1];
    } break;
    case 0x03: { //Reserved, GSM 7 bit default
        res.encoding = CBS_DCS::dcGSM7Bit;
    } break;

    case 0x04: case 0x05: case 0x06: case 0x07: { //General Data Coding indication
        res.compressed = (dcs & BIT_SET(5)) ? true : false;
        res.msgClassDefined = (dcs & BIT_SET(4)) ? true : false;
        if (res.msgClassDefined)
            res.msgClass = (dcs & 0x03);
        res.encoding = _enc_enm[((dcs >> 2) & 0x03)];
    } break;

    case 0x09: { //Message with User Data Header (UDH) structure:
        res.UDHind = res.msgClassDefined = true;
        res.msgClass = (dcs & 0x03);
        res.encoding = _enc_enm[((dcs >> 2) & 0x03)];
    } break;

    case 0x08: case 0x0A: case 0x0B: case 0x0C: case 0x0D: { //reserved
        res.encoding = CBS_DCS::dcReserved;
    } break;

    case 0x0E: { //WAP defined
        res.encoding = CBS_DCS::dcReserved;
    } break;

    case 0x0F: { //Data coding / message handling
        res.msgClassDefined = true;
        res.msgClass = (dcs & 0x03);
        res.encoding = _enc_enm[(dcs >> 2) & 0x01];
    } break;

//  case 0x08: case 0x0A: case 0x0B: case 0x0C: case 0x0D: //reserved
//  case 0x0E: //WAP defined 
    default:
        res.encoding = CBS_DCS::dcReserved;
    }
    return res.encoding;
}

CBS_DCS::TextEncoding  parseCBS_DCS(uint8_t dcs)
{
    CBS_DCS res;
    return parseCBS_DCS(dcs, res);
}

} //cbs

}//namespace smsc

