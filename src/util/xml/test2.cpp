//
// File:   test2.cpp.cc
// Author: igork
//
// Created on February 12, 2004, 6:13 PM
//

#define LIBICONV_PLUG FUCK
#include <stdlib.h>
#include <iconv.h>
#include <iostream>

#include "logger/Logger.h"

using namespace std;

int main(int argc, char** argv) {
  smsc::logger::Logger::Init();
  smsc_log_debug(smsc::logger::Logger::getInstance("test"), "asdasdasd");
  iconv_t h = iconv_open("UTF-8", "KOI8-R");
  if (h == (iconv_t) -1) {
    cerr << "iconv_open failed" << endl;
    return 2;
  }
  const char * const inbuf = "inbuf \xC6\xD9\xD7\xC1 test";
  size_t inbytesleft = strlen(inbuf);
  char outbuf[1024];
  size_t outbytesleft = sizeof(outbuf)/sizeof(char);
  
  const char * const iptr = inbuf;
  char * optr = (char*)outbuf;
  size_t ret = iconv(h, &iptr, &inbytesleft, &optr, &outbytesleft);
  iconv_close(h);
  if (ret == (size_t)-1) {
    cerr << "failed" << endl;
    return (1);
  } else {
    *optr=0;
    cout << inbuf << endl;
    cout << outbuf << endl;
    return (EXIT_SUCCESS);
  }
}

