#ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
#define LOG4CPPINIT_H_INCLUDED_C3A87A6B

#include <string>
#include <log4cpp/Category.hh>

namespace log4cpp {
	class Logger
	{
	public:
		inline static Category & getCategory(const std::string &name);
		static void Init(const std::string & configFileName);
	protected:
		static bool isInitialized;
	};
}

#endif // ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
