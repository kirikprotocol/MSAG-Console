#ifndef ___TEST_SPECIFICATION___
#define ___TEST_SPECIFICATION___

namespace smsc {
  namespace test {
	namespace smpp {
	  struct TestSpecification {
		// тесты для проверки различных видов подключения к SMSC
		static const int BIND_TEST_test_bind = 0;
		static const int BIND_TEST_test_transmitter_receiver_enquire_link = 1;
		// тесты для проверки отсылки и приема SMS-ок
		static const int SMS_TEST_test_sms_for_transmitter_receiver_itself = 100;
		static const int SMS_TEST_test_sms_for_transmitter_receiver = 101;
		static const int SMS_TEST_test_sms_for_transceiver_itself = 102;
		static const int SMS_TEST_test_data_coding = 103;
		static const int SMS_TEST_test_smpp_profiler = 104;
		static const int SMS_TEST_test_long_sms = 105;
	  };
	} //namespace smpp
  } // namespace test
} // namespace smsc

#endif //___TEST_SPECIFICATION___
