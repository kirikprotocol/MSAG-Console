#ifndef ___TEST_SPECIFICATION___
#define ___TEST_SPECIFICATION___

namespace smsc {
  namespace test {
	namespace smpp {
	  struct TestSpecification {
		// ����� ��� �������� ��������� ����� ����������� � SMSC
		static const int BIND_TEST_test_bind = 0;
		static const int BIND_TEST_test_transmitter_receiver_enquire_link = 1;
		// ����� ��� �������� ������� � ������ SMS-��
		static const int SMS_TEST_test_sms_for_transmitter_receiver_itself = 100;
		static const int SMS_TEST_test_sms_for_transceiver_itself = 101;
	  };
	} //namespace smpp
  } // namespace test
} // namespace smsc

#endif //___TEST_SPECIFICATION___
