// CPPUnit includes
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Handler.hpp"
#include "ProtectedCopy.hpp"

namespace smsc {
	namespace test {
		namespace util {

			class A : public ProtectedCopy {
			public:
				int a;
				int b;
				int c;
				int d;

				A() {
					printf("\nClass A constructor without arguments");
					a = b = c = d = 0;
				}

				A(int a) {
					printf("\nClass A constructor with arguments %d", a);
					this->a = a;
					b = c = d = 0;
				}
				A(int a, int b) {
					printf("\nClass A constructor with arguments %d and %d", a, b);
					this->a = a;
					this->b = b;
					c = d = 0;
				}
				A(int a, int b, int c) {
					printf("\nClass A constructor with arguments %d, %d and %d", a, b, c);
					this->a = a;
					this->b = b;
					this->c = c;
					d = 0;
				}
				A(int a, int b, int c, int d) {
					printf("\nClass A constructor with arguments %d, %d, %d and %d", a, b, c, d);
					this->a = a;
					this->b = b;
					this->c = c;
					this->d = d;
				}

				~A() {
					printf("\nClass A destructor with arguments %d, %d, %d and %d", a, b, c, d);
				}
			};

			// test class for handlers
			class HandlerTest : public CppUnit::TestFixture {
				CPPUNIT_TEST_SUITE( HandlerTest );
				CPPUNIT_TEST( testCreation );
				//CPPUNIT_TEST( testAddition );
				//CPPUNIT_TEST_EXCEPTION( testException,  TestException );
				CPPUNIT_TEST_SUITE_END();

			protected:
				void testCreation() {
					A* a = new A();
					Handler<A> h = a;
					printf("\nHandler h : %d, %d, %d, %d", h->a, h->b, h->c, h->d);
					printf("\nComparison a==h : %d", a == h);
					printf("\nComparison h==a : %d", h == a);
					Handler<A> ha = new A(1);
					printf("\nHandler ha : %d, %d, %d, %d", ha->a, ha->b, ha->c, ha->d);
					printf("\nComparison ha==a : %d", ha == a);
					Handler<A> hab = new A(1, 2);
					printf("\nHandler hab : %d, %d, %d, %d", hab->a, hab->b, hab->c, hab->d);
					Handler<A> habc = new A(1, 2, 3);
					printf("\nHandler habc : %d, %d, %d, %d", habc->a, habc->b, habc->c, habc->d);
					Handler<A> habcd = new A(1, 2, 3, 4);
					printf("\nHandler habcd : %d, %d, %d, %d", habcd->a, habcd->b, habcd->c, habcd->d);

					Handler<A> b = new A(11, 22, 33, 44);
					printf("\nHandler b : %d, %d, %d, %d", b->a, b->b, b->c, b->d);

					Handler<A> c(b);
					printf("\nHandler c : %d, %d, %d, %d", c->a, c->b, c->c, c->d);
					printf("\nComparison b==c : %d", b == c);

					ha = habcd;
					printf("\nHandler ha : %d, %d, %d, %d", ha->a, ha->b, ha->c, ha->d);
					printf("\nComparison ha==habcd : %d", ha == habcd);
					printf("\nComparison ha==habc : %d", ha == habc);
				}
			};
		}
	}
}

int main( int argc, char **argv) {
	try {
		// CPP Unit test
		CppUnit::TextUi::TestRunner runner;
		runner.addTest( smsc::test::util::HandlerTest::suite() );
		bool wasSucessful = runner.run();
		return wasSucessful;
		//util::handler::HandlerTest test;
		//test.testCreation();
	} catch (...) {
		printf("Unexpected error !!!");
	}
}



