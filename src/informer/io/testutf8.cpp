#include "UTF8.h"
#include "FileGuard.h"
#include "logger/Logger.h"

using namespace eyeline::informer;

int main()
{
    const char* text = "Русский текст #1";
    const char* fname = "testutf8.log";

    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );

    UTF8 utf;
    TmpBuf<char,8192> buf;
    utf.convertToUcs2( text, strlen(text), buf);

    FileGuard f;
    f.create(fname,0666,true,true);
    f.write(buf.get(),buf.GetPos());
    f.close();

    printf("russian text is written into '%s'\n",fname);
    return 0;
}
