#ifndef ___PROTECTED_COPY_FOR_STUPID___
#define ___PROTECTED_COPY_FOR_STUPID___

// �� ���� ���������� �� ������������ :)
namespace smsc {
  namespace test {
	namespace util {

	  class NoCopy {
	  protected:
		NoCopy(NoCopy& copy) {}
		NoCopy& operator = (NoCopy &copy) { return *this; }
	  public:
		NoCopy() {}
	  };

	  class ProtectedCopy {
	  protected:
		NoCopy noCopy;
	  };
	}
  }
}

#endif
