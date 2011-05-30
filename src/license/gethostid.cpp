#include <unistd.h>
#include <stdio.h>

int main()
{
    const long id = gethostid();
    printf("hostid: %lx\n",id);
    return 0;
}
