#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "recode_dll/recode_dll.h"

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
  char text3[] = "\0\0\0\0\0\0\0\xff\xff\xff\xff AAAA\0";
  //int bytes = ConvertTextTo7Bit(text,sizeof(text),buff7bit,sizeof(buff7bit),CONV_ENCODING_CP1251);
  //printf("converted %d\n",bytes);
  Convert7BitToText(text3,10,text2,sizeof(text2));
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
