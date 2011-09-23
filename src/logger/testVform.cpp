#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <memory>

char * vform(const char* format, va_list args,char* buf,size_t bufsize)
{
  size_t size = bufsize;
  char* buffer = buf;//new char[size];

  try
  {

    while (1) {
      va_list aq;
      va_copy(aq,args);
      int n = ::vsnprintf(buffer, size, format, aq);
      va_end(aq);
  
      // If that worked, return a string.
      if ((n > -1) && (static_cast<size_t>(n) < size)) {
        return buffer;
      }
  
      // Else try again with more space.
      size = (n > -1) ?
        n + 1 :   // ISO/IEC 9899:1999
      size * 2; // twice the old size
  
      if(buffer!=buf)delete [] buffer;
      buffer = new char[size];
    }
  } catch(std::exception& e) {
      sprintf(buf,"out of memory");
      return buf;
  }
}


void callvform2( const char* format, va_list args )
{
    char buf[2048];
    char* msg = vform(format,args,buf,sizeof(buf));
    std::auto_ptr<char>(msg==buf ? 0 : msg);
    puts( msg );
}


void callvform( const char* format, ... )
{
    va_list args;
    va_start(args, format);
    callvform2(format,args);
    va_end(args);
}

int main()
{
    std::string s;
    s.reserve(26000);
    const char* letters = "abcdefhalksjfalksfzxlkguejafkljs";
    int letlen = strlen(letters);
    for ( int i = 0; i < 25108; ++i ) {
        s.push_back(letters[i % letlen]);
    }
    printf("string: %s\n",s.c_str());
    callvform("new routes: %s",s.c_str());
    return 0;
}
