#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "recode_dll.h"

unsigned char  _8bit_2_7bit[256];
unsigned char  _7bit_2_8bit[128];

unsigned char x_7bit_2_8bit[] ={
'@','D',' ','0','¡','P','¿','p',
'£','_','!','1','A','Q','a','q',
'$','F','"','2','B','R','b','r',
'¥','G','#','3','C','S','c','s',
'è','L','¤','4','D','T','d','t',
'é','W','%','5','E','U','e','u',
'ù','P','&','6','F','V','f','v',
'ì','Y','\'','7','G','W','g','w',
'ò','S','(','8','H','X','h','x',
'Ç','Q',')','9','I','Y','i','y',
'\n','X','*',':','J','Z','j','z',
'Ø',0,'+',';','K','Ä','k','ä',
'ø','Æ',',','<','L','Ö','l','ö',
'\r','æ','-','=','M','Ñ','m','ñ',
'Å','ß','.','>','N','Ü','n','ü',
'å','É','/','?','O','§','o','à'
};



int main(int argc, char* argv[])
{
	char text2[256];
  //  char buff7bit[256];
  unsigned char text3[] = {
    0xc4,0xf7,0x9a,0xfe,0x86,0x83,
    0xf2,0xa0,0x66,0x11,0x29,0x05,
    0x81,0x40,0x96,0xe7,0x90,0x52,
    0x04,0x0d,0x85,0xc1,0xb3,0xd8,
    0x23,0x4e,0x82,0x40,0x16,0xe8,
    0xd3,0x43,0x29,0x36,0xc5,0x49,
    0xd0,0xf0,0x09,0x9a,0x41,0x8b,
    0xc8,0x7a,0xb1,0xc9,0x42,0x16,
    0x41,0xc2,0xba,0x8f,0x97,0x07,
    0x81,0x40,0xc4,0x22,0xb2,0x38,
    0x09,0xfd,0x42};
  //int bytes = ConvertTextTo7Bit(text,sizeof(text),buff7bit,sizeof(buff7bit),CONV_ENCODING_CP1251);
  //printf("converted %d\n",bytes);
  Convert7BitToText((char*)text3,sizeof(text3),text2,sizeof(text2));
  printf("result %s\n",text2);

  /*memset(_7bit_2_8bit,'?',256);
  memset(_8bit_2_7bit,(0x15*4),256);
  for ( int i = 0; i<128; ++i ){
    _8bit_2_7bit[x_7bit_2_8bit[(i&15)*8+(i>>4)]] = i;
    _7bit_2_8bit[i] = x_7bit_2_8bit[(i&15)*8+(i>>4)];
  }

  printf("unsigned char  _8bit_2_7bit[256] = {\n");
  for ( int x1 = 0; x1 < 255; ++x1 ){
    printf("0x%x,\n",_8bit_2_7bit[x1]);
  }
  printf("0x%x};\n",_8bit_2_7bit[255]);

  printf("unsigned char  _7bit_2_8bit[128] = {\n");
  for ( int x2 = 0; x2 < 127; ++x2 ){
    printf("0x%x,\n",_7bit_2_8bit[x2]);
  }
  printf("0x%x};\n",_7bit_2_8bit[127]);*/
  return 0;
}
