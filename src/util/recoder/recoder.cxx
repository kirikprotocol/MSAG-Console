#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "recode_dll.h"

unsigned char  _8bit_2_7bit[256];
unsigned char  _7bit_2_8bit[128];

unsigned char x_7bit_2_8bit[] ={
'@','D',' ','0','�','P','�','p',
'�','_','!','1','A','Q','a','q',
'$','F','"','2','B','R','b','r',
'�','G','#','3','C','S','c','s',
'�','L','�','4','D','T','d','t',
'�','W','%','5','E','U','e','u',
'�','P','&','6','F','V','f','v',
'�','Y','\'','7','G','W','g','w',
'�','S','(','8','H','X','h','x',
'�','Q',')','9','I','Y','i','y',
'\n','X','*',':','J','Z','j','z',
'�',0,'+',';','K','�','k','�',
'�','�',',','<','L','�','l','�',
'\r','�','-','=','M','�','m','�',
'�','�','.','>','N','�','n','�',
'�','�','/','?','O','�','o','�'
};



int main(int argc, char* argv[])
{
  char text2[256];
  //  char buff7bit[256];
  unsigned char text3[] = {
    0xd0,0x3d,0xcd,0x66,0x83,0xc4,0x65,0x10,0x3b,0x4c,0x2f,0x03,0xc4,0xe1};
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
