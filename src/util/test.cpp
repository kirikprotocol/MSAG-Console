#include "debug.h"

int main(int argc, char** argv)
{
	int kk=0;
	watch(kk);
	require ( kk != 0 );
}
