#ifndef ___RESPONSE_CHECKER___
#define ___RESPONSE_CHECKER___

#include "Sme.hpp"

namespace smsc {
  namespace test {
	namespace smpp {
	  
	  class ResponseChecker {
		static log4cpp::Category& log; // logger
	  public:
		//�������� ������� �� �������
		static bool checkResponse(uint32_t sequence, uint32_t commandId, QueuedSmeHandler sme, uint32_t timeout);
		//�������� ������� �� ������� bind
		static bool checkBind(uint32_t sequence, int bindType, QueuedSmeHandler sme, uint32_t timeout);		
		//�������� ������� �� ������� unbind
		static bool checkUnbind(uint32_t sequence, QueuedSmeHandler sme, uint32_t timeout);		
		//�������� ������� �� ������� enquire_link
		static bool checkEnquireLink(uint32_t sequence, QueuedSmeHandler sme, uint32_t timeout);	  
	  };

	} // namespace smpp
  } //namespace test
} // namespace smsc

#endif
