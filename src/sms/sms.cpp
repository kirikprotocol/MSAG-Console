#include <stdlib.h>
#include <string.h>

#include "sms.h"

namespace smsc { namespace sms
{

/*void Address::convertSemiToText(void)
{
	for (int i=0; i<lenght; i++)
	{
        for (int j=0; j<2; j++)
		{
			uint8_t ch = ((j) ? (value[i] >> 4) : value[i]) & 0x0F;
            switch (ch)
			{
				case 0x0A: 	
					text[2*i+j] = '*'; break;
				case 0x0B: 	
					text[2*i+j] = '#'; break;
				case 0x0C: 	
					text[2*i+j] = 'a'; break;
				case 0x0D: 	
					text[2*i+j] = 'b'; break;
				case 0x0E: 	
					text[2*i+j] = 'c'; break;
				case 0x0F:	
					text[2*i+j] = '\0';	return;	
				default: 	
					text[2*i+j] = '0'+ch; break;
			}
		}
	}
	text[2*lenght] = '\0';
}*/

char* Body::getDecodedText() 
{
    return 0L;
}

}}


