#ifndef ___SMPP_PROFILE_MANAGER___
#define ___SMPP_PROFILE_MANAGER___

#include "Sme.hpp"

namespace smsc {
  namespace test {
	namespace smpp {

	  typedef smsc::test::util::Handler<smsc::sms::Address> SmsAddressHandler;

	  class SmppProfileManager : smsc::test::util::ProtectedCopy {
        static const char* profileCommands[];
        static const char* profileResponses[];
        smsc::logger::Logger log;
		QueuedSmeHandler sme;
		SmsAddressHandler origAddr;
		SmsAddressHandler profilerAddr;
        uint32_t timeout;
	  public:
		enum ProfileCommand {
		  REPORT_NONE, 
		  REPORT_FULL, 
		  REPORT_FINAL, 
		  LOCALE_RU, 
		  LOCALE_EN, 
		  CP_DEFAULT, 
		  CP_LATIN1, 
		  CP_LATIN1_UCS2, 
		  CP_UCS2, 
		  HIDE_SENDER, 
		  UNHIDE_SENDER
		};
		SmppProfileManager(QueuedSmeHandler esme);
		~SmppProfileManager();
        bool setProfile(ProfileCommand cmd, uint8_t dataCoding=smsc::smpp::DataCoding::BINARY);
		void setOriginatingAddress(const char *addr);
		void setProfilerAddress(const char *addr);
	  };

      typedef smsc::test::util::Handler<SmppProfileManager> SmppProfileManagerHandler;
	}//smpp
  }//namespace test
}//namespace smsc

#endif
