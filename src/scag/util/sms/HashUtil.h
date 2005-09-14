#ifndef _SCAG_SESSION_SMS_HASHUTIL_
#define _SCAG_SESSION_SMS_HASHUTIL_


#include "sms/sms.h"


namespace scag { namespace util { namespace sms {

using smsc::sms::Address;

        class XAddrHashFunc{
        public:
            static uint32_t CalcHash(const Address& key)
            {
                uint32_t retval = key.type^key.plan;
                int i;
                for(i=0;i<key.length;i++)
                {
                    retval=retval*10+(key.value[i]-'0');
                }
                return retval;
            }
	};
}}}																						

#endif
