#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>

const int imax = 100;
const int jmax = 100;

void makestruct( const char* pfx )
{
    for ( int i = 0; i < imax; ++i ) {
        char buf[40];
        sprintf(buf,"%u",i);
        mkdir(buf,0777);
        for ( int j = 0; j < jmax; ++j ) {
            sprintf(buf,"%u/%u%s",i,j,pfx);
            close(creat(buf,0666));
        }
    }
}


void dorename( const char* pfx, const char* sfx )
{
    for ( int i = 0; i < imax; ++i ) {
        for ( int j = 0; j < jmax; ++j ) {
            char buf1[40];
            char buf2[40];
            sprintf(buf1,"%u/%u%s",i,j,pfx);
            sprintf(buf2,"%u/%u%s",i,j,sfx);
            rename(buf1,buf2);
        }
    }
}


int main( int argc, char** argv )
{
    const char* pfx = ".old";
    const char* sfx = ".new";
    if ( argc > 1 ) {
        makestruct(pfx);
        return 0;
    }
    dorename(pfx,sfx);
    dorename(sfx,pfx);
    return 0;
}
