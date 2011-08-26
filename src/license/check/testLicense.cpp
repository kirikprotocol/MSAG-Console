#include <stdio.h>
#include "license.hpp"

int main()
{
    char buf[200];
    smsc::license::check::gethostid(buf,sizeof(buf));
    printf("hostid: %s\n",buf);
    return 0;
}
