#ifndef ___CSV_CPPUNIT_OUTPUTTER___
#define ___CSV_CPPUNIT_OUTPUTTER___

#include <cppunit/Portability.h>
#include <cppunit/Outputter.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestFailure.h>
#include <iostream>

namespace smsc {
	namespace test {
		namespace smpp {

			class CsvOutputter : public CppUnit::Outputter, public smsc::test::util::ProtectedCopy {
				CppUnit::TestResultCollector *resultCollector;
				std::ostream &out;
			public:
				CsvOutputter( CppUnit::TestResultCollector *result,
											std::ostream &stream ) 
				: resultCollector(result), out(stream)
				{
				}

				void write() {
					typedef std::map<CppUnit::Test *,CppUnit::TestFailure*> FailedTests;
					const CppUnit::TestResultCollector::TestFailures &failures = resultCollector->failures();
					const CppUnit::TestResultCollector::Tests &tests = resultCollector->tests();
					std::cout  << "Test Results:" << std::endl;
					std::cout  <<  "Run:  "  <<  resultCollector->runTests()
					<<  "   Failures: "  <<  resultCollector->testFailures()
					<<  "   Errors: "  <<  resultCollector->testErrors() << std::endl;

					FailedTests failedTests;
					CppUnit::TestResultCollector::TestFailures::const_iterator itFailure = failures.begin();
					while ( itFailure != failures.end() ) {
						CppUnit::TestFailure *failure = *itFailure++;
						failedTests.insert(std::make_pair(failure->failedTest(), failure));
						out << failure->failedTestName();
						if (failure->isError()) {
							out << ", Error, " << failure->sourceLine().fileName() << ":" << failure->sourceLine().lineNumber() << std::endl;
						} else {
							out << ", Failed, " << failure->sourceLine().fileName() << ":" << failure->sourceLine().lineNumber() << std::endl;
						}
						std::cout << failure->toString() << std::endl
						<< "        " << failure->sourceLine().fileName() << ":" << failure->sourceLine().lineNumber() << std::endl;
					}

					for ( unsigned int testNumber = 0; testNumber < tests.size(); ++testNumber ) {
						CppUnit::Test *test = tests[testNumber];
						if ( failedTests.find( test ) == failedTests.end() ) {
							out << test->getName() << ", Passed, " << std::endl;
						}
					}
				}
			};
		}	// namespace smpp
	}	// namespace test
}	//  namespace smsc

#endif //___CSV_CPPUNIT_OUTPUTTER___
