#ifndef ___TEST_SPECIFICATION___
#define ___TEST_SPECIFICATION___

namespace smsc {
  namespace test {
	namespace smpp {
	  struct TestSpecification {
		static const int BIND_TEST_test_bind = 0;
		static const int BIND_TEST_test_transmitter_receiver_enquire_link = 1;
	  };
	} //namespace smpp
  } // namespace test
} // namespace smsc

#endif //___TEST_SPECIFICATION___
