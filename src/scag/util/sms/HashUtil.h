#ifndef _SCAG_SESSION_SMS_HASHUTIL_
#define _SCAG_SESSION_SMS_HASHUTIL_


        class XAddrHashFunc{
	        public:
		            static uint32_t CalcHash(const Address& key)
			                {
					                uint32_t retval = key.type^key.plan;
							                int i;
									                for(i=0;i<key.defLength;i++)
											                {
													                    retval=retval*10+(key.addr.value[i]-'0');
															                    }
																	                    return retval;
																			                }
																					        };
																						

#endif
